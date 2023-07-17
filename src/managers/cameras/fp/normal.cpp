#include "managers/cameras/fp/normal.hpp"
#include "managers/cameras/camutil.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "scale/height.hpp"

using namespace RE;

namespace Gts {
	float FirstPerson::GetScaleOverride(bool IsCrawling) {
		float proneFactor = 1.0;
		if (IsCrawling) {
			auto player = GetCameraActor();
			float scale = get_visual_scale(player);
			return scale * this->ProneAdjustment();
		} else {
			return -1.0;
		}
	}
}
