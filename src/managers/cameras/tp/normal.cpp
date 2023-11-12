#include "managers/cameras/tp/normal.hpp"
#include "managers/cameras/camutil.hpp"
#include "managers/GtsSizeManager.hpp"
#include "data/runtime.hpp"

using namespace RE;

namespace Gts {
	NiPoint3 Normal::GetOffset(const NiPoint3& cameraPos) {
		return NiPoint3(
			Runtime::GetFloat("cameraX"),
			Normal::ZOffset,
			Runtime::GetFloat("cameraY")
			);
	}

	NiPoint3 Normal::GetCombatOffset(const NiPoint3& cameraPos) {
		return NiPoint3(
			Runtime::GetFloat("combatCameraX"),
			Normal::ZOffset,
			Runtime::GetFloat("combatCameraY")
			);
	}

	NiPoint3 Normal::GetOffsetProne(const NiPoint3& cameraPos) {
		return NiPoint3(
			Runtime::GetFloat("proneCameraX"),
			Normal::ZOffset,
			Runtime::GetFloat("proneCameraY")
			);
	}

	NiPoint3 Normal::GetCombatOffsetProne(const NiPoint3& cameraPos)  {
		return NiPoint3(
			Runtime::GetFloat("proneCombatCameraX"),
			Normal::ZOffset,
			Runtime::GetFloat("proneCombatCameraY")
			);
	}

	 // fVanityModeMaxDist:Camera Changes The Offset Value We Need So we need to take this value into account;
	void Normal::SetZOff(float Offset) { 
		//The 0.15 was found through testing different fVanityModeMaxDist values
		Normal::ZOffset = Offset - (0.15 * Gts::MaxZoom());
	}

	BoneTarget Normal::GetBoneTarget() {
		auto player = PlayerCharacter::GetSingleton();
		auto& sizemanager = SizeManager::GetSingleton();
		int altMode = Runtime::GetInt("NormalCameraTarget");

		float offset = -45;	

		if (sizemanager.GetActionBool(player, 3)) {
			altMode = 8; // Thigh Sandwich
			offset = 0;
		} else if (sizemanager.GetActionBool(player, 2)) {
			altMode = 9; // Vore
			offset = 0;
		} else if (sizemanager.GetActionBool(player, 4)) {
			altMode = 10; // Vore: Track Right Hand
			offset = 0;
		} else if (sizemanager.GetActionBool(player, 5)) {
			altMode = 11; // L feet
			offset = -10;
		} else if (sizemanager.GetActionBool(player, 6)) {
			altMode = 12; // R Feet
			offset = -10;
		} else if (sizemanager.GetActionBool(player, 0)) {
			altMode = 13; // Thigh Crushing
			offset = -10;
		} else if (sizemanager.GetActionBool(player, 7)) {
			altMode = 14; // Track Left Hand
			offset = 0;
		} else if (sizemanager.GetActionBool(player, 8)) {
			altMode = 15; // Track booty
			offset = -45;
		} else if (sizemanager.GetActionBool(player, 9)) {
			altMode = 5; // Track Breasts
			offset = -15;
		}

		SetZOff(offset);

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
			case 15: {
				return BoneTarget {
				        .boneNames = {
						"NPC L Butt",
						"NPC R Butt",
						"NPC L Foot [Lft ]",
						"NPC R Foot [Rft ]",
					},
				        .zoomScale = 1.25,
				};
			}
		}
		return BoneTarget();
	}
}
