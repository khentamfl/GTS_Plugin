#include "managers/altcamera.hpp"
#include "scale/scale.hpp"
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

		if (ImCrouching >= 1.0) {
			ImProne = true;
		} else {
			ImProne = false;
		}

		UpdateFirstPerson(ImProne); // Update FP camera

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
						auto playerTrans = model->world;
						auto playerTransInve = model->world.Invert();

						// Get Scaled Camera Location
						auto cameraLocation = cameraRoot->world.translate;
						auto targetLocationWorld = playerTrans*((playerTransInve*cameraLocation) * scale);
						auto parent = niCamera->parent;
						NiTransform transform = parent->world.Invert();
						auto targetLocationLocal = transform * targetLocationWorld;

						// Add adjustments
						log::info("Delta: {},{}", deltaX, deltaY);
						targetLocationLocal.x += deltaX * scale;
						targetLocationLocal.y += deltaY * scale;

						// Set Camera
						niCamera->local.translate = targetLocationLocal;
						update_node(niCamera);
					}
				}
			}
		}
	}

	void CameraManager::AdjustUpDown(float amt) {
		deltaY += amt;
	}
	void CameraManager::ResetUpDown() {
		deltaY = 0.0;
	}

	void CameraManager::AdjustLeftRight(float amt) {
		deltaX += amt;
	}
	void CameraManager::ResetLeftRight() {
		deltaX = 0.0;
	}
}
