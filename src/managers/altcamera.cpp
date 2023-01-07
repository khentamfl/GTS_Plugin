#include "managers/altcamera.hpp"
#include "managers/cameras/camutil.hpp"
#include "util.hpp"
#include "data/runtime.hpp"
#include "data/time.hpp"
#include "data/persistent.hpp"
#include "Config.hpp"
#include "node.hpp"
#include "data/time.hpp"

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

		// Handles Transitioning
		if (currentState != this->currentState) {
			if (this->currentState) {
				this->currentState->ExitState();
			}
			if (currentState) {
				currentState->EnterState();
			}
			auto prevState = this->currentState;
			this->currentState = currentState;
			if (prevState) {
				if (currentState) {
					this->transitionState.reset(new TransState(prevState, currentState));
					currentState = this->transitionState.get();
				} else {
					this->transitionState.reset(nullptr);
				}
			} else {
				this->transitionState.reset(nullptr);
			}
		} else {
			if (this->transitionState) {
				if (!this->transitionState->IsDone()) {
					currentState = this->transitionState.get();
				} else {
					this->transitionState.reset(nullptr);
				}
			}
		}

		// Handles updating the camera
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
			NiPoint3 smoothedPlayerLocalOffset = this->smoothPlayerOffset.value;

			// Unsmoothed adjustmnets
			NiPoint3 instant = currentState->GetPlayerLocalOffsetInstant();
			smoothedPlayerLocalOffset += instant;

			// Apply camera scale and offset
			UpdateCamera(this->smoothScale.value, this->smoothOffset.value, smoothedPlayerLocalOffset);

			// Adjust other ini stuff
			if (this->initimer.ShouldRunFrame()) {
				EnsureINIFloat("fMouseWheelZoomIncrement:Camera", Runtime::GetFloat("CameraZoomPrecision"));
				EnsureINIFloat("fMouseWheelZoomSpeed:Camera", Runtime::GetFloat("CameraZoomSpeed")/2);
				EnsureINIFloat("fVanityModeMinDist:Camera", Runtime::GetFloat("MinDistance"));
				EnsureINIFloat("fVanityModeMaxDist:Camera", Runtime::GetFloat("MaxDistance"));
			}
		}
	}

	// Decide which camera state to use
	CameraState* CameraManager::GetCameraState() {
		if (!Runtime::GetBool("EnableCamera") || IsFreeCamera()) {
			return nullptr;
		}
		//=========================================================================

		bool AllowFpCamera = false; // !!!!!Disabled for global release for now!!!!!

		//^^^^^^^===================================================================

		if (IsFirstPerson() && AllowFpCamera) {
			// First Person states
			// 0 is normal
			// 1 is combat
			// 2 is loot
			int FirstPersonMode = Runtime::GetInt("FirstPersonMode");
			switch (FirstPersonMode) {
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
		} else if (!IsFirstPerson()) {
			int cameraMode = Runtime::GetInt("CameraMode");
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
