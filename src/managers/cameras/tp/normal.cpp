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
		if (sizemanager.GetActionBool(player, 3)) {
			altMode = 8; // Thigh Sandwich
		} else if (sizemanager.GetActionBool(player, 2)) {
			altMode = 9; // Vore
		} else if (sizemanager.GetActionBool(player, 4)) {
			altMode = 10; // Vore: Track Hand
		} else if (sizemanager.GetActionBool(player, 5)) {
			altMode = 11; // L feet
		} else if (sizemanager.GetActionBool(player, 6)) {
			altMode = 12; // R Feet
		} else if (sizemanager.GetActionBool(player, 0)) {
			altMode = 13; // Thigh Crushing
		} else if (sizemanager.GetActionBool(player, 7)) {
			altMode = 14; // Grab Attack
		}
		log::info("Alt Mode: {}", altMode);
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
				        .zoomScale = 1.00,
				};
			}
			case 9: {
				return BoneTarget { // Vore
				        .boneNames = {
						"AnimObjectA",
					},
				        .zoomScale = 1.20,
				};
			}
			case 10: {
				return BoneTarget { // Vore Hand
				        .boneNames = {
						"NPC R Hand [RHnd]",
					},
				        .zoomScale = 0.75,
				};
			}
			case 11: {
				return BoneTarget { // L Feet
				        .boneNames = {
						"NPC L Foot [Lft ]",
					},
				        .zoomScale = 0.75,
				};
			}
			case 12: {
				return BoneTarget { // R Feet
				        .boneNames = {
						"NPC R Foot [Rft ]",
					},
				        .zoomScale = 0.75,
				};
			}
			case 13: {
				return BoneTarget { // R feet + L Feet
				        .boneNames = {
						"NPC R PreRearCalf",
						"NPC R Foot [Rft ]",
						"NPC L PreRearCalf",
						"NPC L Foot [Lft ]",
					},
				        .zoomScale = 1.00,
				};
			}
			case 14: {
				return BoneTarget { // L Hand
				        .boneNames = {
						"NPC L Finger02 [LF02]",
					},
				        .zoomScale = 0.60,
				};
			}
		}
		return BoneTarget();
	}
}
