#include "managers/altcamera.hpp"
#include "managers/camera.hpp"
#include "managers/cameras/util.hpp"
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
	void CameraManager::ApplyFirstPerson() {
		bool ImProne = false;
		float ImCrouching = Runtime::GetInt("ImCrouching");
		if (ImCrouching >= 1.0) {
			ImProne = true;
		} else {
			ImProne = false;
		}
		CameraManagerOld::GetSingleton().UpdateFirstPerson(ImProne);
	}

	void CameraManager::CameraUpdate() {
		CameraState* currentState = this->GetCameraState();
		if (IsFirstPerson()) {
			ApplyFirstPerson();
			return; // Do not edit the camera in first person. It causes HUGE issues.
		}
		if (currentState) {
			// Get scale based on camera state
			float scale = currentState->GetScale();

			// Get current camera position in player space
			auto cameraPosLocal  = GetCameraPosLocal();

			// Get either normal or combat offset
			NiPoint3 offset;
			auto player = PlayerCharacter::GetSingleton();
			if (player != nullptr && player->IsWeaponDrawn()) {
				offset = currentState->GetCombatOffset(cameraPosLocal);
			} else {
				offset = currentState->GetOffset(cameraPosLocal);
			}

			offset += this->manualEdit;

			this->smoothOffset.target = offset;
			this->smoothScale.target = scale;

			// Apply camera scale and offset
			ScaleCamera(this->smoothScale.value, this->smoothOffset.value);

			// Adjust other ini stuff
			EnsureINIFloat("fMouseWheelZoomIncrement:Camera", Runtime::GetFloat("CameraZoomPrecision"));
			EnsureINIFloat("fMouseWheelZoomSpeed:Camera", Runtime::GetFloat("CameraZoomSpeed")/2);
			EnsureINIFloat("fVanityModeMinDist:Camera", Runtime::GetFloat("MinDistance"));
			EnsureINIFloat("fVanityModeMaxDist:Camera", Runtime::GetFloat("MaxDistance"));
		}
	}

	// Decide which camera state to use
	CameraState* CameraManager::GetCameraState() {
		if (!Runtime::GetBool("EnableCamera")) {
			return nullptr;
		}
		// 0 is disabled
		// 1 is normal
		// 2 is alt camera
		// 3 is Between Feet
		// 4 is Left Feet
		// 5 is Right Feet
		int cameraMode = Runtime::GetInt("CameraMode");

		bool isProne;
		auto player = PlayerCharacter::GetSingleton();
		if (Runtime::GetBool("ProneEnabled") && player != nullptr && player->IsSneaking()) {
			isProne = true;
		} else {
			isProne = false;
		}

		switch (cameraMode) {
			case 1: {
				if (isProne) {
					return &this->proneState;
				} else {
					return &this->normalState;
				}
			}
			case 2: {
				if (isProne) {
					return &this->altProneState;
				} else {
					return &this->altState;
				}
			}
			case 3: {
				return &this->footState;
			}
			default: {
				return nullptr;
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
