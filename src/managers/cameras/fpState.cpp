#include "managers/cameras/fpState.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "data/persistent.hpp"
#include "managers/cameras/camutil.hpp"

using namespace RE;
using namespace Gts;

namespace  Gts {
	void FirstPersonCameraState::ExitState() {
		// Reset the override
		auto player = GetCameraActor();
		auto playerData = Persistent::GetSingleton().GetData(player);
		if (playerData) {
			playerData->scaleOverride = -1.0;
		}
	}

	float FirstPersonCameraState::GetScale() {
		// Dont need to scale as we follow the head bone in fp
		return 1.0;
	}

	float FirstPersonCameraState::ProneAdjustment() {
		if (!IsCrawling(PlayerCharacter::GetSingleton())) {
			return 1.0;
		}
		return clamp(0.25, 20.0, 3.0 * Runtime::GetFloat("ProneOffsetFP"));
	}

	bool FirstPersonCameraState::PermitManualEdit() {
		return false;
	}
	bool FirstPersonCameraState::PermitTransition() {
		return false;
	}

	bool FirstPersonCameraState::PermitCameraTransforms() {
		return false;
	}
}
