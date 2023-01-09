#include "managers/cameras/tpState.hpp"
#include "data/runtime.hpp"

using namespace RE;
using namespace Gts;

namespace Gts {
	NiPoint3 ThirdPersonCameraState::GetPlayerLocalOffsetProne(const NiPoint3& cameraPosLocal) {
		return this->GetPlayerLocalOffsetProne(cameraPosLocal) + this->ProneAdjustment(cameraPosLocal);
	}

	NiPoint3 ThirdPersonCameraState::ProneAdjustment(const NiPoint3& cameraPos) {
		float proneFactor = 1.0 - Runtime::GetFloat("CalcProne");
		NiPoint3 result = NiPoint3();

		result.z = -cameraPos.z * proneFactor;
		return result;
	}
}
