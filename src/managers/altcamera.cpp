#include "managers/altcamera.hpp"
#include "scale/scale.hpp"
#include "managers/tremor.hpp"
#include "util.hpp"
#include "data/runtime.hpp"
#include "data/persistent.hpp"
#include "Config.hpp"
#include "node.hpp"

using namespace SKSE;
using namespace RE;
using namespace REL;
using namespace Gts;

namespace {
	void SetINIFloat(std::string_view name, float value) {
		auto ini_conf = INISettingCollection::GetSingleton();
		Setting* setting = ini_conf->GetSetting(name);
		if (setting) {
			setting->data.f=value; // If float
			ini_conf->WriteSetting(setting);
		}
	}
	void UpdateThirdPerson() {
		auto camera = PlayerCamera::GetSingleton();
		auto player = PlayerCharacter::GetSingleton();
		if (camera && player) {
			camera->UpdateThirdPerson(player->IsWeaponDrawn());
		}
	}

	void ResetIniSettings() {
		SetINIFloat("fOverShoulderPosX:Camera", 30.0);
		SetINIFloat("fOverShoulderPosY:Camera", 30.0);
		SetINIFloat("fOverShoulderPosZ:Camera", -10.0);
		SetINIFloat("fOverShoulderCombatPosX:Camera", 0.0);
		SetINIFloat("fOverShoulderCombatPosY:Camera", 0.0);
		SetINIFloat("fOverShoulderCombatPosZ:Camera", 20.0);
		SetINIFloat("fVanityModeMaxDist:Camera", 600.0);
		SetINIFloat("fVanityModeMinDist:Camera", 155.0);
		SetINIFloat("fMouseWheelZoomSpeed:Camera", 0.8000000119);
		SetINIFloat("fMouseWheelZoomIncrement:Camera", 0.075000003);
		UpdateThirdPerson();
	}
}

namespace Gts {
	CameraManager& CameraManager::GetSingleton() noexcept {
		static CameraManager instance;
		return instance;
	}

	std::string CameraManager::DebugName() {
		return "CameraManager";
	}

	void CameraManager::Start() {
		ResetIniSettings();
	}

	void CameraManager::UpdateFirstPerson(bool ImProne) {
		auto player = PlayerCharacter::GetSingleton();
		float scale = get_target_scale(player);
		float ProneOffsetFP = 1.0;
		if (player->IsSneaking() == true && ImProne == true) {
			ProneOffsetFP = clamp(0.25, 20.0, 3.0 * Runtime::GetFloat("ProneOffsetFP"));
		}
		set_fp_scale(player, scale, ProneOffsetFP);
	}

	void CameraManager::Update() {
		auto camera = PlayerCamera::GetSingleton();
		auto cameraRoot = camera->cameraRoot.get();
		NiCamera* niCamera = nullptr;

		bool ImProne = false;
		float ImCrouching = Runtime::GetInt("ImCrouching");

		if (ImCrouching >= 1.0) {
			ImProne = true;
		} else {
			ImProne = false;
		}

		UpdateFirstPerson(ImProne); // Update FP camera
		UpdateCamera(ImProne); // Update third person values

		for (auto child: cameraRoot->GetChildren()) {
			NiAVObject* node = child.get();
			log::info("- {}", GetRawName(node));
			if (node) {
				NiCamera* casted = netimmerse_cast<NiCamera*>(node);
				if (casted) {
					niCamera = casted;
					break;
				}
			}
		}
		if (niCamera) {
			auto player = PlayerCharacter::GetSingleton();
			if (player) {
				float scale = get_visual_scale(player);
				if (scale > 1e-4) {
					auto model = player->Get3D(false);
					if (model) {
						float AllowChanges = 1.0;
						if (player && TremorManager::GetSingleton().GetFP()) { // Rough first person fix
							scale = 1.0;
							AllowChanges = 0.0; // Disallow FP edits
						}

						auto playerTrans = model->world;
						auto playerTransInve = model->world.Invert();

						// Get Scaled Camera Location
						auto cameraLocation = cameraRoot->world.translate;

						

						auto targetLocationWorld = playerTrans*((playerTransInve*cameraLocation) * scale);
						auto parent = niCamera->parent;
						NiTransform transform = parent->world.Invert();
						auto targetLocationLocal = transform * targetLocationWorld;
						
						// Add adjustments
						//log::info("Delta: {},{}", deltaX, deltaZ);
						targetLocationLocal.x += ((CameraX + deltaX) * scale) * AllowChanges;
						targetLocationLocal.z += ((CameraZ + deltaZ) * scale) * AllowChanges;

						
						// Set Camera
						niCamera->local.translate = targetLocationLocal;
						update_node(niCamera);
					}
				}
			}
		}
	}

	void CameraManager::AdjustUpDown(float amt) {
		deltaZ += amt;
	}
	void CameraManager::ResetUpDown() {
		deltaZ = 0.0;
	}

	void CameraManager::AdjustLeftRight(float amt) {
		deltaX += amt;
	}
	void CameraManager::ResetLeftRight() {
		deltaX = 0.0;
	}


	void CameraManager::ApplyCameraSettings(float size, float X, float Y, float AltX, float AltY, float MinDistance, float MaxDistance, float usingAutoDistance, bool ImProne, bool IsWeaponDrawn) {
		float cameraYCorrection = 121.0;
		float CalcProne = Runtime::GetFloat("CalcProne");
		float ProneOffsetFP = 1.0;

		if (!IsWeaponDrawn) {
			CameraX = (X * size);
			CameraZ = ((Y * size) - cameraYCorrection) / 10;
		} 

		else if (IsWeaponDrawn) {
			CameraX = (AltX * size);
			CameraZ = ((AltY * size) - cameraYCorrection) / 10;
		}
		if (usingAutoDistance <= 0.0) {
			//SetfVanityModeMinDist(MinDistance * size);
			//SetfVanityModeMaxDist(MaxDistance * size);
		}

		if (PlayerCharacter::GetSingleton()->IsSneaking() == true && ImProne == true) {
				CameraZ *= (1.0 - CalcProne);
			}
		}

	}

	void CameraManager::ApplyFeetCameraSettings(float size, float X, float Y, float AltX, float AltY, float MinDistance, float MaxDistance, float usingAutoDistance, bool ImProne, bool IsWeaponDrawn) {
		float cameraYCorrection = 205.0 * (size * 0.33) + 70;
		float CalcProne = Runtime::GetFloat("CalcProne");
		float ProneOffsetFP = 1.0;

		if (!IsWeaponDrawn) {
			CameraX = (X * size);
			CameraZ = ((Y * size) - cameraYCorrection) / 10;
		} 

		else if (IsWeaponDrawn) {
			CameraX = (AltX * size);
			CameraZ = ((AltY * size) - cameraYCorrection) / 10;
		}
		if (usingAutoDistance <= 0.0) {
			//SetfVanityModeMinDist(MinDistance * size);
			//SetfVanityModeMaxDist(MaxDistance * size);
		}

		if (PlayerCharacter::GetSingleton()->IsSneaking() == true && ImProne == true) {
				CameraZ *= (1.0 - CalcProne);
			}
	}

	void CameraManager::UpdateCamera(bool ImProne) {
		auto player = PlayerCharacter::GetSingleton();
		bool WeaponSheathed = player->IsWeaponDrawn();
		float size = get_target_scale(player);

		float EnableAltCamera = Runtime::GetInt("EnableAltCamera");
		float EnableCamera = Runtime::GetInt("EnableCamera");
		float FeetCamera = Runtime::GetInt("FeetCamera");
		float usingAutoDistance = Runtime::GetInt("usingAutoDistance");
		float MinDistance = Runtime::GetFloat("MinDistance");
		float MaxDistance = Runtime::GetFloat("MaxDistance");
		float CameraZoomSpeed = Runtime::GetFloat("CameraZoomSpeed");
		float CameraZoomPrecision = Runtime::GetFloat("CameraZoomPrecision");
		//////////Normal - Prone
		float proneCameraX = Runtime::GetFloat("proneCameraX");
		float proneCameraY = Runtime::GetFloat("proneCameraY");
		float proneCombatCameraX = Runtime::GetFloat("proneCombatCameraX");
		float proneCombatCameraY = Runtime::GetFloat("proneCombatCameraY");
		/////////Normal - Normal
		float cameraX = Runtime::GetFloat("cameraX");
		float cameraY = Runtime::GetFloat("cameraY");
		float combatCameraX = Runtime::GetFloat("combatCameraX");
		float combatCameraY = Runtime::GetFloat("combatCameraY");
		/////////Alternate - Prone
		float proneCameraAlternateX = Runtime::GetFloat("proneCameraAlternateX");
		float proneCameraAlternateY = Runtime::GetFloat("proneCameraAlternateY");
		float proneCombatCameraAlternateX = Runtime::GetFloat("proneCombatCameraAlternateX");
		float proneCombatCameraAlternateY = Runtime::GetFloat("proneCameraAlternateY");
		////////Alternate - Normal
		float cameraAlternateX = Runtime::GetFloat("cameraAlternateX");
		float cameraAlternateY = Runtime::GetFloat("cameraAlternateY");
		float combatCameraAlternateX = Runtime::GetFloat("combatCameraAlternateX");
		float combatCameraAlternateY = Runtime::GetFloat("combatCameraAlternateY");
		if (EnableCamera < 1.0) {
			CameraX = 0.0;
			CameraZ = 0.0;
			return;
		}

		if (EnableAltCamera >= 3.0) { // Adjustment for Feet Camera
			if (player->IsSneaking() == true && ImProne == true) {
				ApplyFeetCameraSettings(size, proneCameraAlternateX, proneCameraAlternateY, proneCombatCameraAlternateX, proneCombatCameraAlternateY, MinDistance, MaxDistance, usingAutoDistance, ImProne, WeaponSheathed);
			} else {
				ApplyFeetCameraSettings(size, cameraAlternateX, cameraAlternateY, combatCameraAlternateX, combatCameraAlternateY, MinDistance, MaxDistance, usingAutoDistance, ImProne, WeaponSheathed);
			}
		} else if (EnableAltCamera >= 2.0 && EnableAltCamera < 3.0) { // Adjustment for Alternate Camera
			if (player->IsSneaking() == true && ImProne == true) {
				ApplyCameraSettings(size, proneCameraAlternateX, proneCameraAlternateY, proneCombatCameraAlternateX, proneCombatCameraAlternateY, MinDistance, MaxDistance, usingAutoDistance, ImProne, WeaponSheathed);
			} else {
				ApplyCameraSettings(size, cameraAlternateX, cameraAlternateY, combatCameraAlternateX, combatCameraAlternateY, MinDistance, MaxDistance, usingAutoDistance, ImProne, WeaponSheathed);
			}
		} else if (EnableCamera >= 1.0) { // Regular Camera

			if (player->IsSneaking() == true && ImProne == true) {
				ApplyCameraSettings(size, proneCameraX, proneCameraY, proneCombatCameraX, proneCombatCameraY, MinDistance, MaxDistance,usingAutoDistance, ImProne, WeaponSheathed);
			} else {
				ApplyCameraSettings(size, cameraX, cameraY, combatCameraX, combatCameraY, MinDistance, MaxDistance, usingAutoDistance, ImProne, WeaponSheathed);
			}
		}
}

