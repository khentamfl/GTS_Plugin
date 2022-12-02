#include "managers/cameras/alt.hpp"
#include "data/runtime.hpp"

using namespace RE;

namespace Gts {
	NiPoint3 Alt::GetOffset(const NiPoint3& cameraPos) {
		return NiPoint3(
			0.0,
			Runtime::GetFloat("cameraAlternateX"),
			Runtime::GetFloat("cameraAlternateY")
			);
	}

	NiPoint3 Alt::GetCombatOffset(const NiPoint3& cameraPos) {
		return NiPoint3(
			0.0,
			Runtime::GetFloat("combatCameraAlternateX"),
			Runtime::GetFloat("combatCameraAlternateY")
			);
	}
}
