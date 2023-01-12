#include "managers/cameras/fp/combat.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "scale/height.hpp"

using namespace RE;

namespace {
	const float BASE_OVERRIDE = 1.0;
}

namespace Gts {
	float FirstPersonCombat::GetScaleOverride(bool isProne) {
		float proneFactor = 1.0;
		if (isProne) {
			proneFactor = this->ProneAdjustment();
		}
		return BASE_OVERRIDE * proneFactor;
	}
}
