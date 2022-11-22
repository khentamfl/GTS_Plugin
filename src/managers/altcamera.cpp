#include "managers/altcamera.hpp"
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

	void PrintCameraNode() {
		auto camera = PlayerCamera::GetSingleton();
		NiAVObject* root = camera->cameraRoot.get();
		while (root) {
			log::info("Camera: {}", root->name);
			auto parent = root->parent;
			if (parent) {
				root = parent;
			} else {
				break;
			}
		}
	}
	void Experiment01() {
		// Causes player to move with camera controls
		auto camera = PlayerCamera::GetSingleton();
		auto player = PlayerCharacter::GetSingleton();
		auto model = player->GetCurrent3D()->AsNode();
		camera->cameraRoot = NiPointer(model);
	}

	void Experiment02() {
		// CTDS on load
		auto camera = PlayerCamera::GetSingleton();
		auto root = camera->cameraRoot;
		auto parent = root->parent;

		NiPointer<NiAVObject> oldRoot;
		parent->DetachChild(root.get(), oldRoot);

		auto gtsRoot = NiNode::Create();
		parent->AttachChild(oldRoot.get());

		gtsRoot->AttachChild(root.get());
	}

	void Experiment03() {
		auto camera = PlayerCamera::GetSingleton();
		auto third = skyrim_cast<ThirdPersonState>(camera->cameraStates[CameraState::kThirdPerson].get());
		log::info("Third Camera OBJ: {}", GetRawName(third->thirdPersonCameraObj));
		log::info("Third Camera FOV: {}", GetRawName(third->thirdPersonFOVControl));
		log::info("Third Camera animatedBoneName: {}", third->animatedBoneName);
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
		log::info("++Camera Experiemnts");
		ResetIniSettings();
		log::info("+ Pre Experiements");
		PrintCameraNode();
		log::info("-");

		// log::info("+ Experiement 01");
		// Experiment01();
		// log::info("-");

		log::info("+ Experiement 02");
		Experiment02();
		log::info("-");

		log::info("+ Post Experiemnts");
		PrintCameraNode();
		log::info("--Camera Experiemnts");
	}

	void CameraManager::Update() {
	}

	void CameraManager::AdjustUpDown(float amt) {
	}
	void CameraManager::ResetUpDown() {
	}

	void CameraManager::AdjustLeftRight(float amt) {
	}
	void CameraManager::ResetLeftRight() {
	}
}
