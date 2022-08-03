#include "managers/tremor.h"
#include "managers/impact.h"
#include "data/runtime.h"
#include "util.h"

using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace {
	enum Formula {
		Power,
		Smooth,
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

		float scale = impact.effective_scale;
		if (!actor->IsSwimming()) {
			if (actor->IsSprinting()) {
				scale *= 1.2; // Sprinting makes you sound bigger
			} else if (actor->IsSneaking()) {
				scale *= 0.55; // Sneaking makes you sound quieter
			} else if (actor->IsWalking()) {
				scale *= 0.85; // Walking makes you sound quieter
			}
			Foot foot_kind = impact.kind;
			if (foot_kind == Foot::JumpLand) {
				scale *= 1.2; // Jumping makes you sound bigger
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
				float distance_to_camera = unit_to_meter(get_distance_to_camera(node));

				// Camera shakes
				float falloff = soft_core(distance_to_camera, 0.024, 2.0, 0.8, 0.0);
				// Power increases cubically with scale (linearly with volume)
				float n = 3.0;
				float min_shake_scale = 1.2; // Before this no shaking
				float max_shake_scale = 20.0; // After this we have full power shaking

				if (scale < min_shake_scale) return;
				float power = 0.0;

				Formula formula = Formula::Smooth;
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
				}


				float intensity = power * falloff;
				float duration_power = 0.25 * power;
				float duration = duration_power / falloff; // As we fall off we have smaller but longer lasting tremors
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
