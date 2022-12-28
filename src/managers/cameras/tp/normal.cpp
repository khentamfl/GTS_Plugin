#include "managers/cameras/tp/normal.hpp"
#include "data/runtime.hpp"

using namespace RE;

namespace Gts {
	NiPoint3 Normal::GetOffset(const NiPoint3& cameraPos) {
		return NiPoint3(
			Runtime::GetFloat("cameraX"),
			0.0,
			Runtime::GetFloat("cameraY")
			);
	}

	NiPoint3 Normal::GetCombatOffset(const NiPoint3& cameraPos) {
		return NiPoint3(
			Runtime::GetFloat("combatCameraX"),
			0.0,
			Runtime::GetFloat("combatCameraY")
			);
	}

	NiPoint3 Normal::GetOffsetProne(const NiPoint3& cameraPos) {
		log::info("Prone");
		return ProneAdjustment(cameraPos) + NiPoint3(
			Runtime::GetFloat("proneCameraX"),
			0.0,
			Runtime::GetFloat("proneCameraY")
			);
	}

	NiPoint3 Normal::GetCombatOffsetProne(const NiPoint3& cameraPos)  {
		log::info("Prone Combat");
		return ProneAdjustment(cameraPos) + NiPoint3(
			Runtime::GetFloat("proneCombatCameraX"),
			0.0,
			Runtime::GetFloat("proneCombatCameraY")
			);
	}
}
