#include "managers/tremor.h"
#include "managers/impact.h"
#include "data/runtime.h"
#include "data/persistent.h"
#include "data/transient.h"
#include "util.h"

using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace {
	enum Formula {
		Power,
		Smooth,
		SoftCore,
	};
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
				float power_multi = 1.0;
				switch (foot_kind) {
					case Foot::Left:
					case Foot::Right:
					case Foot::Front:
					case Foot::Back:
						break;
					case Foot::JumpLand:
						power_multi = 2.0;
						break;
					default:
						return;
						break;
				}
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
				SoftPotential falloff_sp {
					.k = 0.024,
					.n = 2.0,
					.s = 0.8,
					.o = 0.0,
					.a = 0.0,
				};
				float falloff = soft_core(distance * 1.5, falloff_sp);
				// Power increases cubically with scale (linearly with volume)
				float n = 3.0;
				float min_shake_scale = 1.2; // Before this no shaking
				float max_shake_scale = 20.0; // After this we have full power shaking

				if (scale < min_shake_scale) return;
				float power = 0.0;

				Formula formula = Formula::SoftCore;
				switch (formula) {
					case Formula::Power:
					{
						float k = 1.0/pow(max_shake_scale - min_shake_scale, n);
						power = k*pow(scale - min_shake_scale, n) * power_multi;
					}
					case Formula::Smooth:
					{
						power = smootherstep(min_shake_scale, max_shake_scale, scale);
					}
					case Formula::SoftCore:
					{
						SoftPotential softness {
							.k = 0.054,
							.n = 0.26,
							.s = 1.0,
							.o = 1.2,
							.a = -1.17,
						};

						power = soft_power(scale, softness);
					}
				}


				float intensity = power * falloff * tremor_scale;
				float duration_power = 0.25 * (scale * 0.15 + 0.85) * tremor_scale;//power * tremor_scale;
				float duration = duration_power * intensity; // falloff; // As we fall off we have smaller but longer lasting tremors
				if (duration >= 1.20)
				{duration = 1.20;}
				if (intensity > 0.05 && duration > 0.05) {
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
