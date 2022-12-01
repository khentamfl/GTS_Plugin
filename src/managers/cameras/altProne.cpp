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
			Runtime::GetFloat("proneCameraAlternateX"),
			0.0,
			Runtime::GetFloat("proneCameraAlternateY")
			);
	}

	NiPoint3 AltProne::GetCombatOffset(const NiPoint3& cameraPos) {
		return ProneAdjustment(cameraPos) + NiPoint3(
			Runtime::GetFloat("proneCombatCameraAlternateX"),
			0.0,
			Runtime::GetFloat("proneCombatCameraAlternateY")
			);
	}
}
