#include "managers/cameras/fpState.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"

using namespace RE;
using namespace Gts;

namespace  Gts {
	float FirstPersonCameraState::ProneAdjustment() {
		return clamp(0.25, 20.0, 3.0 * Runtime::GetFloat("ProneOffsetFP"));
	}

	bool FirstPersonCameraState::PermitManualEdit() {
		return false;
	}
	bool FirstPersonCameraState::PermitTransition() {
		return false;
	}
}
