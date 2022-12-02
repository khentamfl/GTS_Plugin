#include "managers/cameras/prone.hpp"
#include "data/runtime.hpp"

using namespace RE;
using namespace Gts;

namespace {
	NiPoint3 ProneAdjustment(const NiPoint3& cameraPos) {
		float proneFactor = Runtime::GetFloat("CalcProne");
		log::info("proneFactor: {}", proneFactor);
		NiPoint3 result = NiPoint3();

		result.z = -cameraPos.z * proneFactor;
		return result;
	}
}

namespace Gts {
	NiPoint3 Prone::GetOffset(const NiPoint3& cameraPos) {
		log::info("Prone");
		return ProneAdjustment(cameraPos) + NiPoint3(
			0.0,
			Runtime::GetFloat("proneCameraX"),
			Runtime::GetFloat("proneCameraY")
			);
	}

	NiPoint3 Prone::GetCombatOffset(const NiPoint3& cameraPos)  {
		log::info("Prone Combat");
		return ProneAdjustment(cameraPos) + NiPoint3(
			0.0,
			Runtime::GetFloat("proneCombatCameraX"),
			Runtime::GetFloat("proneCombatCameraY")
			);
	}
}
