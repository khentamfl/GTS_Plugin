#include "managers/cameras/tp/alt.hpp"
#include "data/runtime.hpp"

using namespace RE;

namespace Gts {
	NiPoint3 Alt::GetOffset(const NiPoint3& cameraPos) {
		return NiPoint3(
			Runtime::GetFloat("cameraAlternateX"),
			0.0,
			Runtime::GetFloat("cameraAlternateY")
			);
	}

	NiPoint3 Alt::GetCombatOffset(const NiPoint3& cameraPos) {
		return NiPoint3(
			Runtime::GetFloat("combatCameraAlternateX"),
			0.0,
			Runtime::GetFloat("combatCameraAlternateY")
			);
	}

	NiPoint3 Alt::GetOffsetProne(const NiPoint3& cameraPos) {
		return NiPoint3(
			Runtime::GetFloat("proneCameraAlternateX"),
			0.0,
			Runtime::GetFloat("proneCameraAlternateY")
			);
	}

	NiPoint3 Alt::GetCombatOffsetProne(const NiPoint3& cameraPos) {
		return NiPoint3(
			Runtime::GetFloat("proneCombatCameraAlternateX"),
			0.0,
			Runtime::GetFloat("proneCombatCameraAlternateY")
			);
	}

	std::vector<std::string> Alt::GetBoneTargets() {
		int altMode = Runtime::GetInt("AltCameraTarget");
		switch (altMode) {
			case 0: {
				return {};
			}
			case 1: {
				return {
				        "NPC Spine [Spn0]",
				};
			}
			case 2: {
				return {
				        "NPC Spine1 [Spn1]",
				};
			}
			case 3: {
				return {
				        "NPC L Breast",
				        "NPC R Breast",
				};
			}
			case 4: {
				return {
				        "L Breast02",
				        "R Breast02",
				};
			}
			case 5: {
				return {
				        "NPC Neck [Neck]",
				};
			}
			case 6: {
				return {
				        "NPC L Butt",
				        "NPC R Butt",
				};
			}
		}
		return {};
	}
}
