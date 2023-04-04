#include "managers/cameras/tp/alt.hpp"
#include "managers/GtsSizeManager.hpp"
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

	BoneTarget Alt::GetBoneTarget() {
		auto player = PlayerCharacter::GetSingleton();
		auto& sizemanager = SizeManager::GetSingleton();
		int altMode = Runtime::GetInt("AltCameraTarget");
		if (sizemanager.GetActionBool(player, 1)) {
			altmode = 8; // Thigh Sandwich
		} else if (sizemanager.GetActionBool(player, 2)) {
			altmode = 9; // Vore
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
						"NPC R Calf [RClf]",
						"NPC R RearThigh",
					},
				        .zoomScale = 0.75,
				};
			}
			case 9: {
				return BoneTarget { // Vore
				        .boneNames = {
						"NPC R Hand [RHnd]",
					},
				        .zoomScale = 1.50,
				};
			}
		}
		return BoneTarget();
	}
}
