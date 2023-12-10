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
		auto player = PlayerCharacter::GetSingleton();
		float value = 1.0;//std::clamp(Runtime::GetFloat("ProneOffsetFP"), 0.10f, 1.0f);
		if (IsCrawling(player)) {
			value = std::clamp(Runtime::GetFloat("ProneOffsetFP"), 0.10f, 1.0f);
		}
		return value;
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
