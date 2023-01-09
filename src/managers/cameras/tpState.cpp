#include "managers/cameras/tpState.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"

using namespace RE;
using namespace Gts;

namespace Gts {
	NiPoint3 ThirdPersonCameraState::GetPlayerLocalOffsetInstant(const NiPoint3& cameraPos) {
		NiPoint3 pos = NiPoint3();
		auto player = PlayerCharacter::GetSingleton();
		if (player) {
			auto scale = get_visual_scale(player);
			pos += this->ProneAdjustment(cameraPos)*scale;
		}

		return pos;
	}

	NiPoint3 ThirdPersonCameraState::ProneAdjustment(const NiPoint3& cameraPos) {
		float proneFactor = 1.0 - Runtime::GetFloat("CalcProne");
		NiPoint3 result = NiPoint3();

		result.z = -cameraPos.z * proneFactor;
		return result;
	}
}
