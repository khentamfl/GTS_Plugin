#include "managers/cameras/state.hpp"
#include "managers/GtsSizeManager.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"

using namespace RE;
using namespace Gts;

namespace {
	NiPoint3 ProneAdjustment(const NiPoint3& cameraPos) {
		float proneFactor = 1.0 - Runtime::GetFloat("CalcProne");

		NiPoint3 result = NiPoint3();

		result.z = -cameraPos.z * proneFactor;
		return result;
	}
}

namespace Gts {
	float CameraState::GetScale() {
		auto player = PlayerCharacter::GetSingleton();
		auto 
		if (!player) {
			return 1.0;
		}
		return get_target_scale(player) * SizeManager::GetRaceScale(player);
	}

	NiPoint3 CameraState::GetOffset(const NiPoint3& cameraPosLocal) {
		return NiPoint3(0.0, 0.0, 0.0);
	}
	NiPoint3 CameraState::GetOffsetProne(const NiPoint3& cameraPosLocal) {
		return this->GetOffset(cameraPosLocal);
	}
	NiPoint3 CameraState::GetOffset(const NiPoint3& cameraPosLocal, bool isProne) {
		if (isProne) {
			return this->GetOffsetProne(cameraPosLocal);
		} else {
			return this->GetOffset(cameraPosLocal);
		}
	}

	NiPoint3 CameraState::GetCombatOffset(const NiPoint3& cameraPosLocal) {
		return NiPoint3(0.0, 0.0, 0.0);
	}
	NiPoint3 CameraState::GetCombatOffsetProne(const NiPoint3& cameraPosLocal) {
		return this->GetCombatOffset(cameraPosLocal);
	}
	NiPoint3 CameraState::GetCombatOffset(const NiPoint3& cameraPosLocal, bool isProne) {
		if (isProne) {
			return this->GetCombatOffsetProne(cameraPosLocal);
		} else {
			return this->GetCombatOffset(cameraPosLocal);
		}
	}

	NiPoint3 CameraState::GetPlayerLocalOffset(const NiPoint3& cameraPosLocal) {
		return NiPoint3(0.0, 0.0, 0.0);
	}
	NiPoint3 CameraState::GetPlayerLocalOffsetProne(const NiPoint3& cameraPosLocal) {
		return this->GetPlayerLocalOffset(cameraPosLocal);
	}
	NiPoint3 CameraState::GetPlayerLocalOffset(const NiPoint3& cameraPosLocal, bool isProne) {
		if (isProne) {
			return this->GetPlayerLocalOffsetProne(cameraPosLocal);
		} else {
			return this->GetPlayerLocalOffset(cameraPosLocal);
		}
	}

	bool CameraState::PermitManualEdit() {
		return true;
	}
}
