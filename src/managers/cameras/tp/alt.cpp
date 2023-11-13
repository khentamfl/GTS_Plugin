#include "managers/cameras/camutil.hpp"
#include "managers/cameras/tp/alt.hpp"
#include "managers/GtsSizeManager.hpp"
#include "data/runtime.hpp"

using namespace RE;

namespace Gts {
	NiPoint3 Alt::GetOffset(const NiPoint3& cameraPos) {
		return NiPoint3(
			Runtime::GetFloat("cameraAlternateX"),
			0,
			Runtime::GetFloat("cameraAlternateY")
			);
	}

	NiPoint3 Alt::GetCombatOffset(const NiPoint3& cameraPos) {
		return NiPoint3(
			Runtime::GetFloat("combatCameraAlternateX"),
			0,
			Runtime::GetFloat("combatCameraAlternateY")
			);
	}

	NiPoint3 Alt::GetOffsetProne(const NiPoint3& cameraPos) {
		return NiPoint3(
			Runtime::GetFloat("proneCameraAlternateX"),
			0,
			Runtime::GetFloat("proneCameraAlternateY")
			);
	}

	NiPoint3 Alt::GetCombatOffsetProne(const NiPoint3& cameraPos) {
		return NiPoint3(
			Runtime::GetFloat("proneCombatCameraAlternateX"),
			0,
			Runtime::GetFloat("proneCombatCameraAlternateY")
			);
	}

	// fVanityModeMaxDist:Camera Changes The Offset Value We Need So we need to take this value into account;
    void Alt::SetZOff(float Offset) {
        // The 0.15 was found through testing different fVanityModeMaxDist values
        //Alt::ZOffset = Offset - (0.15 * Gts::MaxZoom());
    }

	BoneTarget Alt::GetBoneTarget() {
		auto player = PlayerCharacter::GetSingleton();
		auto& sizemanager = SizeManager::GetSingleton();
		int altMode = Runtime::GetInt("AltCameraTarget");

		float offset = -45;		

		if (sizemanager.GetActionBool(player, 3)) {
			altMode = 8; // Thigh Sandwich
			offset = 0;
		} else if (sizemanager.GetActionBool(player, 2)) {
			altMode = 9; // Vore
			offset = 0;
		} else if (sizemanager.GetActionBool(player, 4)) {
			altMode = 10; // Vore: Track Hand
			offset = 0;
		} else if (sizemanager.GetActionBool(player, 5)) {
			altMode = 11; // L Feet
			offset = -10;
		} else if (sizemanager.GetActionBool(player, 6)) {
			altMode = 12; // R feet
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
				        .zoomScale = 1.50,
				};
			}
			case 10: {
				return BoneTarget { // Vore Hand
				        .boneNames = {
						"NPC R Hand [RHnd]",
					},
				        .zoomScale = 0.50,
				};
			}
			case 11: {
				return BoneTarget { // L feet
				        .boneNames = {
						"NPC L Foot [Lft ]",
					},
				        .zoomScale = 0.75,
				};
			}
			case 12: {
				return BoneTarget { // R feet
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
				        .zoomScale = 0.50,
				};
			}
			case 15: {
				return BoneTarget {
				        .boneNames = {
						"NPC L Butt",
						"NPC R Butt",
						"NPC L RearThigh",
						"NPC R RearThigh",
					},
				        .zoomScale = 1.25,
				};
			}
		}
		return BoneTarget();
	}
}
