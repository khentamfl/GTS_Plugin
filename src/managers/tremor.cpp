#include "managers/tremor.h"
#include "managers/impact.h"
#include "data/runtime.h"
#include "data/persistent.h"
#include "data/transient.h"
#include "Config.h"
#include "util.h"

using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace {
	enum Formula {
		Power,
		Smooth,
		SoftCore,
		Linear,
		Unknown,
	};

	float falloff_calc(float x, float half_power) {
		float n_falloff = 2.0;
		return 1/(1+pow(pow(1/0.5-1,n_falloff)*(x)/half_power,half_power));
	}
}

namespace Gts {
	TremorManager& TremorManager::GetSingleton() noexcept {
		static TremorManager instance;
		return instance;
	}

	void TremorManager::OnImpact(const Impact& impact) {
		if (!impact.actor) return;
		auto actor = impact.actor;

		float tremor_scale;
		if (actor->formID == 0x14) {
			tremor_scale = Persistent::GetSingleton().tremor_scale;
		} else {
			tremor_scale = Persistent::GetSingleton().npc_tremor_scale;
		}

		if (tremor_scale < 1e-5) {
			return;
		}

		float scale = impact.effective_scale;
		if (!actor->IsSwimming()) {
			if (actor->IsSprinting()) {
				scale *= 1.35; // Sprinting makes you seem bigger
			} else if (actor->IsSneaking()) {
				scale *= 0.55; // Sneaking makes you seem quieter
			} else if (actor->IsWalking()) {
				scale *= 0.85; // Walking makes you seem quieter
			}
			Foot foot_kind = impact.kind;
			if (foot_kind == Foot::JumpLand) {
				scale *= 2.5; // Jumping makes you seem bigger
			}
			auto actor_data = Transient::GetSingleton().GetData(actor);
			if (actor_data) {
				scale *= actor_data->get_hh_bonus_factor();
			}

			for (NiAVObject* node: impact.nodes) {
				float distance = 0.0;
				if (actor->formID == 0x14) {
					distance = unit_to_meter(get_distance_to_camera(node));
				} else {
					auto point_a = node->world.translate;
					auto point_b = PlayerCharacter::GetSingleton()->GetPosition();
					auto delta = point_a - point_b;

					distance = unit_to_meter(delta.Length());
				}

				// Camera shakes
				auto config = Config::GetSingleton().GetTremor();

				float falloff = falloff_calc(distance, config.GetHalfScale());

				float min_shake_scale = config.GetMinScale(); // Before this no shaking
				float max_shake_scale = config.GetMaxScale(); // After this we have full power shaking
				float power_at_min = config.GetPowerAtMin(); // Power at minimum scale and zero distance will be much lower than this at 2m due to falloff
				float power_at_max = config.GetPowerAtMax(); // Power at maximum scale

				if (scale < min_shake_scale) return;
				float power = 0.0;


				// The equation to use
				//
				// FullTesting graph: https://www.desmos.com/calculator/qazgd0awcx
				std::map<std::string, Formula> s_mapStringToForumlua =
				{
					{ "linear", Formula::Linear },
					{ "smoothstep", Formula::Smooth },
					{ "softcore", Formula::SoftCore },
					{ "cubic", Formula::Power }
				};
				Formula formula = s_mapStringToForumlua[config.GetMethod()];

				switch (formula) {
					case Formula::Power:
					{
						// Power increases cubically with scale (linearly with volume)
						float n = 3.0;
						float k = 1.0/pow(max_shake_scale - min_shake_scale, n);
						power = k*pow(scale - min_shake_scale, n)*(power_at_max-power_at_min) + power_at_min;
					}
					case Formula::Smooth:
					{
						// Smooth step
						power = smootherstep(min_shake_scale, max_shake_scale, scale)*(power_at_max-power_at_min) + power_at_min;
					}
					case Formula::SoftCore:
					{
						// A root like softpower
						// https://www.desmos.com/calculator/p7vfatfljg
						float n = 0.24;
						float s = 1.0;
						float a = -1.17;
						// Altered to maintain the powerat/min/max
						float o = -pow(pow(power_at_min-a, s)-1, 1/(n*s))*(max_shake_scale-min_shake_scale)/pow(pow(power_at_max-a, s)-1.0, 1/(n*s))+min_shake_scale;
						float k = pow(pow(power_at_max-a, s)-1.0,1/(n*s))/(max_shake_scale-o);
						SoftPotential softness {
							.k = k,
							.n = n,
							.s = s,
							.o = o,
							.a = a,
						};

						power = soft_power(scale, softness);
					}
					case Formula::Linear:
					{
						// Linear
						float m = (power_at_max-power_at_min)/(max_shake_scale-min_shake_scale);
						float c = power_at_min-(power_at_max-power_at_min)/(max_shake_scale-min_shake_scale)*min_shake_scale;
						power = m*scale + c;
					}
					default: {
						log::info("Tremor method invalid");
					}
				}

				float intensity = power * falloff * tremor_scale;

				float duration = power * tremor_scale * 0.5;
				duration = smootherstep(duration, 0.2, 1.2);

				log::info("Shake values at scale {}:", scale);
				log::info("  - intensity: {}", intensity);
				log::info("  - power: {}", power);
				log::info("  - falloff: {}", falloff);
				log::info("  - tremor_scale: {}", tremor_scale);
				log::info("  - duration: {}", duration);
				log::info("  - distance: {}", distance);
				if (intensity > 0.01 && duration > 0.01) {
					shake_camera(actor, intensity, duration);

					float left_shake = intensity;
					float right_shake = intensity;
					if (actor->formID == 0x14) {
						switch (foot_kind) {
							case Foot::Left:
							case Foot::Front:
								right_shake = 0.0;
								break;
							case Foot::Right:
							case Foot::Back:
								left_shake = 0.0;
								break;
						}
					}
					shake_controller(left_shake, right_shake, duration);
				}
			}
		}
	}
}
