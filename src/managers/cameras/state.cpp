#include "managers/cameras/state.hpp"
#include "managers/GtsSizeManager.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"

using namespace RE;
using namespace Gts;

namespace Gts {
	void CameraState::EnterState() {
	}
	void CameraState::ExitState() {
	}

	float CameraState::GetScale() {
		auto player = PlayerCharacter::GetSingleton();
		float racescale = SizeManager::GetSingleton().GetRaceScale(player);
		float result = get_visual_scale(player) * racescale;
		if (!player) {
			return 1.0;
		}
		return result;
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
	bool CameraState::PermitTransition() {
		return true;
	}
}
