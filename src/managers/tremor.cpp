#include "managers/tremor.h"
#include "managers/impact.h"
#include "data/runtime.h"
#include "util.h"

using namespace SKSE;
using namespace RE;
using namespace Gts;

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
				float a = min_shake_scale;
				float k = 1.0/pow(scale - a, n);
				float power = k*pow(scale - a, n) * power_multi;

				float intensity = power * falloff;
				float duration_power = 0.25 * power;
				float duration = duration_power * falloff;
				if (intensity > 0.05 && duration > 0.05) {
					this->shake_camera(actor, intensity, duration);

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
					this->shake_controller(left_shake, right_shake, duration);
				}
			}
		}
	}


	void TremorManager::Process() {
		std::unique_lock lock(this->_lock);
		if (this->camera_tremor.process.exchange(false)) {
			Gts::shake_camera(this->camera_tremor.actor, this->camera_tremor.intensity, this->camera_tremor.duration);
		}
		if (this->controller_tremor.process.exchange(false)) {
			Gts::shake_controller(this->controller_tremor.intensity_l, this->controller_tremor.intensity_r, this->controller_tremor.duration);
		}
	}

	void TremorManager::shake_camera(Actor* actor, float intensity, float duration) {
		std::unique_lock lock(this->_lock);
		if (this->camera_tremor.process.load()) {
			intensity = std::max(this->camera_tremor.intensity, intensity);
			duration = std::max(this->camera_tremor.duration, duration);
			if (get_distance_to_camera(actor) > get_distance_to_camera(this->camera_tremor.actor)) {
				actor = this->camera_tremor.actor;
			}
		}
		this->camera_tremor.process.store(true);
		this->camera_tremor.intensity = intensity;
		this->camera_tremor.duration = duration;
		this->camera_tremor.actor = actor;
	}
	void TremorManager::shake_controller(float intensity_l, float intensity_r, float duration) {
		std::unique_lock lock(this->_lock);
		if (this->controller_tremor.process.load()) {
			intensity_l = std::max(this->controller_tremor.intensity_l, intensity_l);
			intensity_r = std::max(this->controller_tremor.intensity_r, intensity_r);
			duration = std::max(this->controller_tremor.duration, duration);
		}
		this->controller_tremor.process.store(true);
		this->controller_tremor.intensity_l = intensity_l;
		this->controller_tremor.intensity_r = intensity_r;
		this->controller_tremor.duration = duration;
	}

}
