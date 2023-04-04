#include "managers/cameras/tp/normal.hpp"
#include "managers/GtsSizeManager.hpp"
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

	BoneTarget Normal::GetBoneTarget() {
		auto player = PlayerCharacter::GetSingleton();
		auto& sizemanager = SizeManager::GetSingleton();
		int altMode = Runtime::GetInt("NormalCameraTarget");
		if (sizemanager.GetActionBool(player, 1)) {
			altMode = 8; // Thigh Sandwich
		} else if (sizemanager.GetActionBool(player, 2)) {
			altMode = 9; // Vore
		} else if (sizemanager.GetActionBool(player, 3)) {
			altMode = 10; // Vore
		}
		switch (altMode) {
			case 0: {
				return BoneTarget();
			}
			case 1: {
				return BoneTarget {
				        .boneNames = {
						"NPC Spine2 [Spn2]",
						"NPC Neck [Neck]",
					},
				        .zoomScale = 0.75,
				};
			}
			case 2: {
				return BoneTarget {
				        .boneNames = {
						"NPC R Clavicle [RClv]",
						"NPC L Clavicle [LClv]",
					},
				        .zoomScale = 0.75,
				};
			}
			case 3: {
				return BoneTarget {
				        .boneNames = {
						"NPC L Breast",
						"NPC R Breast",
					},
				        .zoomScale = 0.75,
				};
			}
			case 4: {
				return BoneTarget {
				        .boneNames = {
						"L Breast02",
						"R Breast02",
					},
				        .zoomScale = 0.75,
				};
			}
			case 5: {
				return BoneTarget {
				        .boneNames = {
						"L Breast03",
						"R Breast03",
					},
				        .zoomScale = 0.75,
				};
			}
			case 6: {
				return BoneTarget {
				        .boneNames = {
						"NPC Neck [Neck]",
					},
				        .zoomScale = 0.75,
				};
			}
			case 7: {
				return BoneTarget {
				        .boneNames = {
						"NPC L Butt",
						"NPC R Butt",
					},
				        .zoomScale = 0.75,
				};
			}
			case 8: {
				return BoneTarget { // Thigh Sandwich
				        .boneNames = {
						"AnimObjectA",
					},
				        .zoomScale = 1.75,
				};
			}
			case 9: {
				return BoneTarget { // Vore
				        .boneNames = {
						"AnimObjectA",
					},
				        .zoomScale = 1.50,
				};
			}
		}
		return BoneTarget();
	}
}
