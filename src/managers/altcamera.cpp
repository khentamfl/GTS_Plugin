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

	std::string PrintParents(NiAVObject* node, std::string_view prefix = "") {
		std::string result = "";
		std::string name = node->name.c_str();
		std::string rawName = GetRawName(node);
		result += std::format("{}- {}", prefix, name);
		result += std::format("\n{}  = {}", prefix, rawName);
		if (node->parent) {
			result += std::format("\n{}", PrintParents(node->parent, std::format("{}  ", prefix)));
		}
		return result;
	}


	std::string PrintNode(NiAVObject* node, std::string_view prefix = "") {
		std::string result = "";
		std::string name = node->name.c_str();
		std::string rawName = GetRawName(node);
		result += std::format("{}- {}", prefix, name);
		result += std::format("\n{}  = {}", prefix, rawName);
		auto niNode = node->AsNode();
		if (niNode) {
			for (auto child: niNode->GetChildren()) {
				if (child) {
					result += std::format("\n{}", PrintNode(child.get(), std::format("  {}", prefix)));
				}
			}
		}
		return result;
	}

	void Experiment03() {
		auto camera = PlayerCamera::GetSingleton();
		auto third = skyrim_cast<ThirdPersonState*>(camera->cameraStates[CameraState::kThirdPerson].get());
		log::info("Third Camera OBJ: {}", GetRawName(third->thirdPersonCameraObj));
		log::info("{}", PrintNode(third->thirdPersonCameraObj));
		log::info("{}", PrintParents(third->thirdPersonCameraObj));
		log::info("Third Camera FOV: {}", GetRawName(third->thirdPersonFOVControl));
		log::info("{}", PrintNode(third->thirdPersonFOVControl));
		log::info("{}", PrintParents(third->thirdPersonFOVControl));
		log::info("Third Camera animatedBoneName: {}", third->animatedBoneName);
	}

	void Experiment04() {
		auto camera = PlayerCamera::GetSingleton();
		auto third = skyrim_cast<ThirdPersonState*>(camera->cameraStates[CameraState::kThirdPerson].get());
		// log::info("Cam node pos: {}::{}", Vector2Str(third->thirdPersonCameraObj->world.translate), Vector2Str(third->thirdPersonCameraObj->local.translate));
		third->thirdPersonCameraObj->local.translate *= get_visual_scale(PlayerCharacter::GetSingleton());
	}

	void Experiment05() {
		auto camera = PlayerCamera::GetSingleton();
		auto third = skyrim_cast<ThirdPersonState*>(camera->cameraStates[CameraState::kThirdPerson].get());
		// log::info("Cam node pos: {}::{}", Vector2Str(third->thirdPersonCameraObj->world.translate), Vector2Str(third->thirdPersonCameraObj->local.translate));
		auto player = PlayerCharacter::GetSingleton();
		if (player) {
			float scale = get_visual_scale(player);
			if (scale > 1e-4) {
				auto model = player->Get3D(false);
				if (model) {
					auto playerTrans = model->world;
					auto playerTransInve = model->world.Invert();

					// Third Person Camera Object
					auto cameraLocation = third->thirdPersonCameraObj->world.translate;
					auto targetLocationWorld = playerTrans*((playerTransInve*cameraLocation) * scale);
					auto parent = third->thirdPersonCameraObj->parent;
					NiTransform transform = parent->world.Invert();
					auto targetLocationLocal = transform * targetLocationWorld;
					third->thirdPersonCameraObj->local.translate = targetLocationLocal;
					update_node(third->thirdPersonCameraObj);

					// Third Person Camera FOV Object
					cameraLocation = third->thirdPersonFOVControl->world.translate;
					targetLocationWorld = playerTrans*((playerTransInve*cameraLocation) * scale);
					parent = third->thirdPersonFOVControl->parent;
					transform = parent->world.Invert();
					targetLocationLocal = transform * targetLocationWorld;
					third->thirdPersonFOVControl->local.translate = targetLocationLocal;
					update_node(third->thirdPersonFOVControl);
				}
			}
		}
	}

	void Experiment06() {
		auto camera = PlayerCamera::GetSingleton();
		auto cameraRoot = camera->cameraRoot.get();
		log::info("{}", PrintNode(cameraRoot));
		log::info("{}", PrintParents(cameraRoot));
	}

	void Experiment07() {
		auto camera = PlayerCamera::GetSingleton();
		auto cameraRoot = camera->cameraRoot.get();
		NiCamera* niCamera = nullptr;
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
			log::info("Camera found");
		}
	}

	void Experiment08() {
		auto camera = PlayerCamera::GetSingleton();
		auto cameraRoot = camera->cameraRoot.get();

		auto player = PlayerCharacter::GetSingleton();
		if (player) {
			float scale = get_visual_scale(player);
			if (scale > 1e-4) {
				auto model = player->Get3D(false);
				if (model) {
					auto playerTrans = model->world;
					auto playerTransInve = model->world.Invert();

					// Camera Root Object
					auto cameraLocation = cameraRoot->world.translate;
					auto targetLocationWorld = playerTrans*((playerTransInve*cameraLocation) * scale);
					auto parent = cameraRoot->parent;
					NiTransform transform = parent->world.Invert();
					auto targetLocationLocal = transform * targetLocationWorld;
					cameraRoot->local.translate = targetLocationLocal;
					update_node(cameraRoot);
				}
			}
		}
	}

	void Experiment09() {
		auto camera = PlayerCamera::GetSingleton();
		auto cameraRoot = camera->cameraRoot.get();
		NiCamera* niCamera = nullptr;
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

						// Camera Root Object
						auto cameraLocation = niCamera->world.translate;
						auto targetLocationWorld = playerTrans*((playerTransInve*cameraLocation) * scale);
						auto parent = niCamera->parent;
						NiTransform transform = parent->world.Invert();
						auto targetLocationLocal = transform * targetLocationWorld;
						niCamera->local.translate = targetLocationLocal;
						update_node(niCamera);
					}
				}
			}
		}
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
		log::info("+ Experiement 07");
		Experiment07();
		log::info("-");
	}

	void CameraManager::Update() {
		Experiment09();
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
