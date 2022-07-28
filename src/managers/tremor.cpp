#include "managers/tremor.h"
#include "managers/impact.h"
#include "data/runtime.h"

using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace {
	void do_shakes(NiAVObject* node, const Foot& foot_kind, const float& scale) {
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
		// 1.0 Meter ~= 20% Power
		// 0.5 Meter ~= 50% Power
		float falloff = soft_core(distance_to_camera, 0.024, 2.0, 0.8, 0.0);
		// Power increases cubically with scale (linearly with volume)
		float n = 3.0;
		float min_shake_scale = 1.2; // Before this no shaking
		float max_shake_scale = 20.0; // After this we have full power shaking
		float a = min_shake_scale;
		float k = 1.0/pow(scale - a, n);
		float power = k*pow(scale - a, n) * power_multi;

		float intensity = power * falloff;
		float duration_power = 0.25 * power;
		float duration = duration_power * falloff;
		if (intensity > 0.05 && duration > 0.05) {
			shake_camera(nullptr, intensity, duration);
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

namespace Gts {
	TremorManager& TremorManager::GetSingleton() noexcept {
		static TremorManager instance;
		return instance;
	}

	void TremorManager::OnImpact(const Impact& impact) {
		if (!impact.actor) return;
		auto actor = impact.actor;

		float scale = impact.effective_scale;
		float minimal_size = 4.0;
		if (scale > minimal_size && !actor->IsSwimming()) {
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
				do_shakes(node, impact.kind, scale);
			}
		}
	}

}
