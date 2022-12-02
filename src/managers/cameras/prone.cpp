#include "managers/cameras/prone.hpp"
#include "data/runtime.hpp"

using namespace RE;
using namespace Gts;

namespace {
	NiPoint3 ProneAdjustment(const NiPoint3& cameraPos) {
		float proneFactor = 1.0 - Runtime::GetFloat("CalcProne");
		log::info("proneFactor: {}", proneFactor);
		NiPoint3 result = NiPoint3();

		result.z = -cameraPos.z * proneFactor;
		log::info("Adjsuting Z by {}", result.z);
		return result;
	}
}

namespace Gts {
	NiPoint3 Prone::GetOffset(const NiPoint3& cameraPos) {
		log::info("Prone");
		return ProneAdjustment(cameraPos) + NiPoint3(
			Runtime::GetFloat("proneCameraX"),
			0.0,
			Runtime::GetFloat("proneCameraY")
			);
	}

	NiPoint3 Prone::GetCombatOffset(const NiPoint3& cameraPos)  {
		log::info("Prone Combat");
		return ProneAdjustment(cameraPos) + NiPoint3(
			Runtime::GetFloat("proneCombatCameraX"),
			0.0,
			Runtime::GetFloat("proneCombatCameraY")
			);
	}
}
