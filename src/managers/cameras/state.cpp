#include "managers/cameras/state.hpp"
#include "scale/scale.hpp"

using namespace RE;

namespace Gts {
	float CameraState::GetScale() {
		auto player = PlayerCharacter::GetSingleton();
		if (!player) {
			return 1.0;
		}
		return get_visual_scale(player);
	}

	NiPoint3 CameraState::GetOffset(const NiPoint3& cameraPosLocal) {
		return NiPoint3(0.0, 0.0, 0.0);
	}

	NiPoint3 CameraState::GetCombatOffset(const NiPoint3& cameraPosLocal) {
		return NiPoint3(0.0, 0.0, 0.0);
	}

	NiPoint3 CameraState::GetPlayerLocalOffset(const NiPoint3& cameraPosLocal) {
		return NiPoint3(0.0, 0.0, 0.0);
	}

	bool CameraState::PermitManualEdit() {
		return true;
	}
}
