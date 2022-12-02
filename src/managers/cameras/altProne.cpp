#include "managers/cameras/altProne.hpp"
#include "data/runtime.hpp"

using namespace RE;
using namespace Gts;

namespace {
	NiPoint3 ProneAdjustment(const NiPoint3& cameraPos) {
		float proneFactor = Runtime::GetFloat("CalcProne");

		NiPoint3 result = NiPoint3();

		result.z = -cameraPos.z * proneFactor;
		return result;
	}
}

namespace Gts {
	NiPoint3 AltProne::GetOffset(const NiPoint3& cameraPos) {
		return ProneAdjustment(cameraPos) + NiPoint3(
			0.0,
			Runtime::GetFloat("proneCameraAlternateX"),
			Runtime::GetFloat("proneCameraAlternateY")
			);
	}

	NiPoint3 AltProne::GetCombatOffset(const NiPoint3& cameraPos) {
		return ProneAdjustment(cameraPos) + NiPoint3(
			0.0,
			Runtime::GetFloat("proneCombatCameraAlternateX"),
			Runtime::GetFloat("proneCombatCameraAlternateY")
			);
	}
}
