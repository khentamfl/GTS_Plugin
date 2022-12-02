#include "managers/cameras/normal.hpp"
#include "data/runtime.hpp"

using namespace RE;

namespace Gts {
	NiPoint3 Normal::GetOffset(const NiPoint3& cameraPos) {
		return NiPoint3(
			0.0,
			Runtime::GetFloat("cameraX"),
			Runtime::GetFloat("cameraY")
			);
	}

	NiPoint3 Normal::GetCombatOffset(const NiPoint3& cameraPos) {
		return NiPoint3(
			0.0,
			Runtime::GetFloat("combatCameraX"),
			Runtime::GetFloat("combatCameraY")
			);
	}
}
