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
		return NiPoint3(
			Runtime::GetFloat("proneCameraX"),
			0.0,
			Runtime::GetFloat("proneCameraY")
			);
	}

	NiPoint3 Normal::GetCombatOffsetProne(const NiPoint3& cameraPos)  {
		log::info("Prone Combat");
		return NiPoint3(
			Runtime::GetFloat("proneCombatCameraX"),
			0.0,
			Runtime::GetFloat("proneCombatCameraY")
			);
	}

	std::vector<std::string> Normal::GetBoneTargets() {
		int altMode = Runtime::GetInt("NormalCameraTarget");
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
