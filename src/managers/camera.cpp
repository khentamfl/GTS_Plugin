#include "managers/camera.hpp"
#include "scale/scale.hpp"
#include "util.hpp"
#include "data/runtime.hpp"
#include "data/persistent.hpp"

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
	float GetINIFloat(std::string_view name) {
		auto ini_conf = INISettingCollection::GetSingleton();
		Setting* setting = ini_conf->GetSetting(name);
		if (setting) {
			return setting->data.f;
		}
		return -1.0;
	}

	void UpdateThirdPerson() {
		auto camera = PlayerCamera::GetSingleton();
		auto player = PlayerCharacter::GetSingleton();
		if (camera && player) {
			camera->UpdateThirdPerson(player->IsWeaponDrawn());
		}
	}
}

namespace Gts {
	CameraManager& CameraManager::GetSingleton() noexcept {
		static CameraManager instance;
		return instance;
	}

	void CameraManager::SetfOverShoulderPosX(float value) {
		auto camera = PlayerCamera::GetSingleton();
		auto player = PlayerCharacter::GetSingleton();
		if (camera) {
			TESCameraState* camera_state = camera->cameraStates[CameraStates::kThirdPerson].get();
			if (camera_state && player->IsWeaponDrawn() == false) {
				ThirdPersonState* third_person_state = static_cast<ThirdPersonState*>(camera_state);
				third_person_state->posOffsetExpected.x = value;
			}
		}
	}
	float CameraManager::GetfOverShoulderPosX() {
		auto camera = PlayerCamera::GetSingleton();
		auto player = PlayerCharacter::GetSingleton();
		TESCameraState* camera_state = camera->cameraStates[CameraStates::kThirdPerson].get();
		ThirdPersonState* third_person_state = static_cast<ThirdPersonState*>(camera_state);
		return third_person_state->posOffsetExpected.x;
	}

	void CameraManager::SetfOverShoulderPosY(float value) {
		auto camera = PlayerCamera::GetSingleton();
		auto player = PlayerCharacter::GetSingleton();

		if (camera && player->IsWeaponDrawn() == false) {
			TESCameraState* camera_state = camera->cameraStates[CameraStates::kThirdPerson].get();
			if (camera_state && player->IsWeaponDrawn() == false) {
				ThirdPersonState* third_person_state = static_cast<ThirdPersonState*>(camera_state);
				third_person_state->posOffsetExpected.y = value;
			}
		}
	}
	float CameraManager::GetfOverShoulderPosY() {
		auto camera = PlayerCamera::GetSingleton();
		auto player = PlayerCharacter::GetSingleton();
		TESCameraState* camera_state = camera->cameraStates[CameraStates::kThirdPerson].get();
		ThirdPersonState* third_person_state = static_cast<ThirdPersonState*>(camera_state);
		return third_person_state->posOffsetExpected.y;
	}

	void CameraManager::SetfOverShoulderPosZ(float value) {
		auto camera = PlayerCamera::GetSingleton();
		auto player = PlayerCharacter::GetSingleton();
		if (camera) {
			TESCameraState* camera_state = camera->cameraStates[CameraStates::kThirdPerson].get();
			if (camera_state && player->IsWeaponDrawn() == false) {
				ThirdPersonState* third_person_state = static_cast<ThirdPersonState*>(camera_state);
				third_person_state->posOffsetExpected.z = value;
			}
		}
	}
	float CameraManager::GetfOverShoulderPosZ() {
		auto camera = PlayerCamera::GetSingleton();
		auto player = PlayerCharacter::GetSingleton();
		TESCameraState* camera_state = camera->cameraStates[CameraStates::kThirdPerson].get();
		ThirdPersonState* third_person_state = static_cast<ThirdPersonState*>(camera_state);
		return third_person_state->posOffsetExpected.z;
	}
	

	void CameraManager::SetfOverShoulderCombatPosX(float value) {
		auto camera = PlayerCamera::GetSingleton();
		auto player = PlayerCharacter::GetSingleton();
		if (camera) {
			TESCameraState* camera_state = camera->cameraStates[CameraStates::kThirdPerson].get();
			if (camera_state && player->IsWeaponDrawn()) {
				ThirdPersonState* third_person_state = static_cast<ThirdPersonState*>(camera_state);
				third_person_state->posOffsetExpected.x = value;
			}
		}
	}

	float CameraManager::GetfOverShoulderCombatPosX() {
		auto camera = PlayerCamera::GetSingleton();
		auto player = PlayerCharacter::GetSingleton();
		TESCameraState* camera_state = camera->cameraStates[CameraStates::kThirdPerson].get();
		ThirdPersonState* third_person_state = static_cast<ThirdPersonState*>(camera_state);
		return third_person_state->posOffsetExpected.x;
	}
	

	void CameraManager::SetfOverShoulderCombatPosY(float value) {
		auto camera = PlayerCamera::GetSingleton();
		auto player = PlayerCharacter::GetSingleton();
		if (camera) {
			TESCameraState* camera_state = camera->cameraStates[CameraStates::kThirdPerson].get();
			if (camera_state && player->IsWeaponDrawn()) {
				ThirdPersonState* third_person_state = static_cast<ThirdPersonState*>(camera_state);
				third_person_state->posOffsetExpected.y = value;
			}
		}
		
	}
	
	float CameraManager::GetfOverShoulderCombatPosY() {
		auto camera = PlayerCamera::GetSingleton();
		auto player = PlayerCharacter::GetSingleton();
		TESCameraState* camera_state = camera->cameraStates[CameraStates::kThirdPerson].get();
		ThirdPersonState* third_person_state = static_cast<ThirdPersonState*>(camera_state);
		return third_person_state->posOffsetExpected.y;
	}

	void CameraManager::SetfOverShoulderCombatPosZ(float value) {
		auto camera = PlayerCamera::GetSingleton();
		auto player = PlayerCharacter::GetSingleton();
		if (camera) {
			TESCameraState* camera_state = camera->cameraStates[CameraStates::kThirdPerson].get();
			if (camera_state && player->IsWeaponDrawn()) {
				ThirdPersonState* third_person_state = static_cast<ThirdPersonState*>(camera_state);
				third_person_state->posOffsetExpected.z = value;
			}
		}
	}
	
	float CameraManager::GetfOverShoulderCombatPosZ() {
		auto camera = PlayerCamera::GetSingleton();
		auto player = PlayerCharacter::GetSingleton();
		TESCameraState* camera_state = camera->cameraStates[CameraStates::kThirdPerson].get();
		ThirdPersonState* third_person_state = static_cast<ThirdPersonState*>(camera_state);
		return third_person_state->posOffsetExpected.z;
	}

	void CameraManager::SetfVanityModeMaxDist(float value) {
		SetINIFloat("fVanityModeMaxDist:Camera", value);
	}
	float CameraManager::GetfVanityModeMaxDist() {
		return GetINIFloat("fVanityModeMaxDist:Camera");
	}

	void CameraManager::SetfVanityModeMinDist(float value) {
		SetINIFloat("fVanityModeMinDist:Camera", value);
	}
	float CameraManager::GetfVanityModeMinDist() {
		return GetINIFloat("fVanityModeMinDist:Camera");
	}

	void CameraManager::SetfMouseWheelZoomSpeed(float value) {
		SetINIFloat("fMouseWheelZoomSpeed:Camera", value);
	}

	void CameraManager::SetfMouseWheelZoomIncrement(float value) {
		SetINIFloat("fMouseWheelZoomIncrement:Camera", value);
	}

	float CameraManager::GetfMouseWheelZoomSpeed() {
		return GetINIFloat("fMouseWheelZoomSpeed:Camera");
	}



	void CameraManager::ApplyCameraSettings(float size, float X, float Y, float AltX, float AltY, float MinDistance, float MaxDistance, float usingAutoDistance, bool ImProne) {
		auto& runtime = Runtime::GetSingleton();
		float cameraYCorrection = 121.0;
		float UpDown = 1.0; float Side = 1.0;
		float CalcProne = runtime.CalcProne->value;

		SetfOverShoulderPosX((X + Side) * size);
		SetfOverShoulderPosZ((Y + UpDown) * size - cameraYCorrection);

		SetfOverShoulderCombatPosX((AltX + Side) * size);
		SetfOverShoulderCombatPosZ(((AltY + UpDown) * size) - cameraYCorrection);

		SetfOverShoulderPosX(((X + Side) * size));
		SetfOverShoulderPosZ(((Y + UpDown) * size) - cameraYCorrection);

		SetfOverShoulderCombatPosX((AltX + Side) * size);
		SetfOverShoulderCombatPosZ((AltY + UpDown) * size - cameraYCorrection);
		if (usingAutoDistance <= 0.0) {
			SetfVanityModeMinDist(MinDistance * size);
			SetfVanityModeMaxDist(MaxDistance * size);
		}

		if (PlayerCharacter::GetSingleton()->IsSneaking() == true && ImProne == true) {
			float ProneCalc = CameraManager::GetfOverShoulderPosZ(); //Utility.getINIFloat("fOverShoulderPosZ:Camera")
			float ProneCalcC = CameraManager::GetfOverShoulderCombatPosZ(); //Utility.getINIFloat("fOverShoulderCombatPosZ:Camera")
			CameraManager::SetfOverShoulderPosZ(ProneCalc * runtime.CalcProne->value); //Utility.setINIFloat("fOverShoulderPosZ:Camera", ProneCalc * CalcProne2)
			CameraManager::SetfOverShoulderCombatPosZ(ProneCalcC * runtime.CalcProne->value); //Utility.setINIFloat("fOverShoulderCombatPosZ:Camera", ProneCalcC * CalcProne2)
		}
	}

	void CameraManager::ApplyFeetCameraSettings(float size, float X, float Y, float AltX, float AltY, float MinDistance, float MaxDistance, float usingAutoDistance, bool ImProne) {
		auto& runtime = Runtime::GetSingleton();
		float cameraYCorrection2 = 205.0 * (size * 0.33) + 70;
		float CalcProne2 = runtime.CalcProne->value;
		float UpDown = 1.0; float Side = 1.0;

		CameraManager::SetfOverShoulderPosX(((X + Side) * size));
		CameraManager::SetfOverShoulderPosZ(((Y + UpDown) * size) - cameraYCorrection2);

		SetfOverShoulderCombatPosX((AltX + Side) * size);
		SetfOverShoulderCombatPosZ((AltY + UpDown) * size - cameraYCorrection2);
		if (usingAutoDistance <= 0.0) {
			SetfVanityModeMinDist(MinDistance * size);
			SetfVanityModeMaxDist(MaxDistance * size);
		}

		if (PlayerCharacter::GetSingleton()->IsSneaking() == true && ImProne == true) {
			float ProneCalc = CameraManager::GetfOverShoulderPosZ(); //Utility.getINIFloat("fOverShoulderPosZ:Camera")
			float ProneCalcC = CameraManager::GetfOverShoulderCombatPosZ(); //Utility.getINIFloat("fOverShoulderCombatPosZ:Camera")
			CameraManager::SetfOverShoulderPosZ(ProneCalc * runtime.CalcProne->value); //Utility.setINIFloat("fOverShoulderPosZ:Camera", ProneCalc * CalcProne2)
			CameraManager::SetfOverShoulderCombatPosZ(ProneCalcC * runtime.CalcProne->value); //Utility.setINIFloat("fOverShoulderCombatPosZ:Camera", ProneCalcC * CalcProne2)
		}
	}

	// Run every frame
	void CameraManager::Update() {
		auto player = PlayerCharacter::GetSingleton();
		auto& runtime = Runtime::GetSingleton();

		auto Camera = PlayerCamera::GetSingleton();
		float CameraX = Camera->pos.x;
		float CameraY = Camera->pos.y;
		float CameraZ = Camera->pos.z;


		bool ImProne = false;
		float size = get_visual_scale(player);
		float ScaleMethod = 0.0;
		SizeMethod method = Persistent::GetSingleton().size_method;
		if (method == SizeMethod::ModelScale) {
			ScaleMethod = 0.0;
		} else if (method == SizeMethod::RootScale) {
			ScaleMethod = 1.0;
		} else if (method == SizeMethod::RefScale) {
			ScaleMethod = 2.0;
		}

		float EnableCamera = runtime.EnableCamera->value;
		float EnableAltCamera = runtime.EnableAltCamera->value;
		float FeetCamera = runtime.FeetCamera->value;
		float usingAutoDistance = runtime.usingAutoDistance->value;
		float ImCrouching = runtime.ImCrouching->value;
		float MinDistance = runtime.MinDistance->value;
		float MaxDistance = runtime.MaxDistance->value;
		float CameraZoomSpeed = runtime.CameraZoomSpeed->value;
		float CameraZoomPrecision = runtime.CameraZoomPrecision->value;
		//////////Normal - Prone
		float proneCameraX = runtime.proneCameraX->value;
		float proneCameraY = runtime.proneCameraY->value;
		float proneCombatCameraX = runtime.proneCombatCameraX->value;
		float proneCombatCameraY = runtime.proneCombatCameraY->value;
		/////////Normal - Normal
		float cameraX = runtime.cameraX->value;
		float cameraY = runtime.cameraY->value;
		float combatCameraX = runtime.combatCameraX->value;
		float combatCameraY = runtime.combatCameraY->value;
		//------------------------------------------------------------------------------------------------------------------------
		/////////Alternate - Prone
		float proneCameraAlternateX = runtime.proneCameraAlternateX->value;
		float proneCameraAlternateY = runtime.proneCameraAlternateY->value;
		float proneCombatCameraAlternateX = runtime.proneCombatCameraAlternateX->value;
		float proneCombatCameraAlternateY = runtime.proneCameraAlternateY->value;
		////////Alternate - Normal
		float cameraAlternateX = runtime.cameraAlternateX->value;
		float cameraAlternateY = runtime.cameraAlternateY->value;
		float combatCameraAlternateX = runtime.combatCameraAlternateX->value;
		float combatCameraAlternateY = runtime.combatCameraAlternateY->value;


		if (ImCrouching >= 1.0) {
			ImProne = true;
		} else {
			ImProne = false;
		}
		if (EnableCamera < 1.0) {
			return;
		} else if (MinDistance < -200) {
			MinDistance = -200;
		}
		if (MaxDistance < 50) {
			MaxDistance = 50;
		}


		if (FeetCamera >= 1.0) { // Adjustment for Feet Camera
			SetfMouseWheelZoomIncrement(CameraZoomPrecision);
			SetfMouseWheelZoomSpeed(CameraZoomSpeed/2);
			if (player->IsSneaking() == true && ImProne == true) {
				CameraManager::ApplyFeetCameraSettings(size, proneCameraAlternateX, proneCameraAlternateY, proneCombatCameraAlternateX, proneCombatCameraAlternateY, MinDistance, MaxDistance, usingAutoDistance, ImProne);
			} else {
				CameraManager::ApplyFeetCameraSettings(size, cameraAlternateX, cameraAlternateY, combatCameraAlternateX, combatCameraAlternateY, MinDistance, MaxDistance, usingAutoDistance, ImProne);
			}
		} else if (EnableAltCamera >= 1.0) { // Adjustment for Alternate Camera
			SetfMouseWheelZoomIncrement(CameraZoomPrecision);
			SetfMouseWheelZoomSpeed(CameraZoomSpeed/2);
			if (player->IsSneaking() == true && ImProne == true) {
				CameraManager::ApplyCameraSettings(size, proneCameraAlternateX, proneCameraAlternateY, proneCombatCameraAlternateX, proneCombatCameraAlternateY, MinDistance, MaxDistance, usingAutoDistance, ImProne);
			} else {
				CameraManager::ApplyCameraSettings(size, cameraAlternateX, cameraAlternateY, combatCameraAlternateX, combatCameraAlternateY, MinDistance, MaxDistance, usingAutoDistance, ImProne);
			}
		} else if (EnableCamera >= 1.0) { // Regular Camera
			SetfMouseWheelZoomIncrement(CameraZoomPrecision);
			SetfMouseWheelZoomSpeed(CameraZoomSpeed/2);
			if (player->IsSneaking() == true && ImProne == true) {
				CameraManager::ApplyCameraSettings(size, proneCameraX, proneCameraY, proneCombatCameraX, proneCombatCameraY, MinDistance, MaxDistance,usingAutoDistance, ImProne);
			} else {
				CameraManager::ApplyCameraSettings(size, cameraX, cameraY, combatCameraX, combatCameraY, MinDistance, MaxDistance, usingAutoDistance, ImProne);
			}

		}
	}
}

