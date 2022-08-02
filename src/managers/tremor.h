#pragma once
// Module that handles footsteps
#include <SKSE/SKSE.h>
#include "managers/impact.h"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	struct CameraTremorCache {
		float intensity;
		float duration;
		Actor* actor;
		std::atomic_bool process;
	};
	struct ControllerTremorCache {
		float intensity_l;
		float intensity_r;
		float duration;
		std::atomic_bool process;
	};
	class TremorManager {
		public:
			[[nodiscard]] static TremorManager& GetSingleton() noexcept;

			void OnImpact(const Impact& impact);
			void Process();

			void shake_camera(Actor* actor, float intensity, float duration);
			void shake_controller(float left_intensity, float right_intensity, float duration);

		private:
			mutable std::mutex _lock;

			CameraTremorCache camera_tremor = CameraTremorCache {
				.intensity = 0.0,
				.duration = 0.0,
				.actor = nullptr,
				.process = std::atomic_bool(false),
			};
			ControllerTremorCache controller_tremor = ControllerTremorCache {
				.intensity_l = 0.0,
				.intensity_r  = 0.0,
				.duration = 0.0,
				.process = std::atomic_bool(false),
			};
	};
}
