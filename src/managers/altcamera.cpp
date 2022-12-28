#include "managers/altcamera.hpp"
#include "managers/camera.hpp"
#include "managers/cameras/camutil.hpp"
#include "util.hpp"
#include "data/runtime.hpp"
#include "data/time.hpp"
#include "data/persistent.hpp"
#include "Config.hpp"
#include "node.hpp"

using namespace SKSE;
using namespace RE;
using namespace REL;
using namespace Gts;



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

	void CameraManager::CameraUpdate() {
		CameraState* currentState = this->GetCameraState();
		if (currentState) {
			bool isProne;
			auto player = PlayerCharacter::GetSingleton();
			if (Runtime::GetBool("ProneEnabled") && player != nullptr && player->IsSneaking()) {
				isProne = true;
			} else {
				isProne = false;
			}

			// Get scale based on camera state
			float scale = currentState->GetScale();

			// Get current camera position in player space
			auto cameraPosLocal  = GetCameraPosLocal();

			// Get either normal or combat offset
			NiPoint3 offset;
			if (player != nullptr && player->IsWeaponDrawn()) {
				offset = currentState->GetCombatOffset(cameraPosLocal, isProne);
			} else {
				offset = currentState->GetOffset(cameraPosLocal, isProne);
			}

			NiPoint3 playerLocalOffset = currentState->GetPlayerLocalOffset(cameraPosLocal, isProne);

			if (currentState->PermitManualEdit()) {
				offset += this->manualEdit;
			}

			this->smoothOffset.target = offset;
			this->smoothScale.target = scale;
			this->smoothPlayerOffset.target = playerLocalOffset;

			// Apply camera scale and offset
			UpdateCamera(this->smoothScale.value, this->smoothOffset.value, this->smoothPlayerOffset.value);

			// Adjust other ini stuff
			EnsureINIFloat("fMouseWheelZoomIncrement:Camera", Runtime::GetFloat("CameraZoomPrecision"));
			EnsureINIFloat("fMouseWheelZoomSpeed:Camera", Runtime::GetFloat("CameraZoomSpeed")/2);
			EnsureINIFloat("fVanityModeMinDist:Camera", Runtime::GetFloat("MinDistance"));
			EnsureINIFloat("fVanityModeMaxDist:Camera", Runtime::GetFloat("MaxDistance"));
		}
	}

	// Decide which camera state to use
	CameraState* CameraManager::GetCameraState() {
		auto playercamera = PlayerCamera::GetSingleton();
		if (!Runtime::GetBool("EnableCamera") || playercamera->currentState == playercamera->cameraStates[CameraState::kFree]) {
			return nullptr;
		}
		int cameraMode = Runtime::GetInt("CameraMode");

		if (IsFirstPerson()) {
			// First Person states
			// 0 is normal
			// 1 is combat
			// 2 is loot
			int FirstPersonMode = Runtime::GetInt("FirstPersonMode"); // TODO: Fix detection
			switch (cameraMode) {
				case 0: {
					return &this->fpState;
				}
				case 1: {
					return &this->fpCombatState;
				}
				case 2: {
					return &this->fpLootState;
				}
				default: {
					return nullptr;
				}
			}
		} else {
			// Third Person states
			// 0 is disabled
			// 1 is normal
			// 2 is alt camera
			// 3 is Between Feet
			// 4 is Left Feet
			// 5 is Right Feet
			switch (cameraMode) {
				case 1: {
					return &this->normalState;
				}
				case 2: {
					return &this->altState;
				}
				case 3: {
					return &this->footState;
				}
				case 4: {
					return &this->footLState;
				}
				case 5: {
					return &this->footRState;
				}
				default: {
					return nullptr;
				}
			}
		}
		return nullptr;
	}

	void CameraManager::AdjustUpDown(float amt) {
		this->manualEdit.z += amt;
	}
	void CameraManager::ResetUpDown() {
		this->manualEdit.z = 0.0;
	}

	void CameraManager::AdjustLeftRight(float amt) {
		this->manualEdit.x += amt;
	}
	void CameraManager::ResetLeftRight() {
		this->manualEdit.x = 0.0;
	}
}
