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

	NiNode* EnsureGtsRoot(NiNode* cameraRoot) {
		const char* ROOT_NAME = "GTSRoot";
		auto currentParent = cameraRoot->parent;
		if (currentParent) {
			if (currentParent->name != ROOT_NAME) {
				log::info("Insterting new node");
				auto parentIndex = cameraRoot->parentIndex;
				auto newParent = NiNode::Create();
				newParent->name = ROOT_NAME;
				NiPointer<NiAVObject> camera;
				currentParent->SetAt(parentIndex, cameraRoot, camera);
				camera.get()->IncRefCount();
				newParent->AttachChild(camera.get());
				update_node(cameraRoot);
				update_node(newParent);
				update_node(currentParent);
				update_node(camera.get());
				return newParent;
			}
		}
		return nullptr;
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

	void CameraManager::Update() {
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
			auto gtsRoot = EnsureGtsRoot(cameraRoot);
			if (gtsRoot) {
				auto gtsParent = gtsRoot->parent;
				if (gtsParent) {
					auto player = PlayerCharacter::GetSingleton();
					if (player) {
						float scale = get_visual_scale(player);
						if (scale > 1e-4) {
							auto model = player->Get3D(false);
							if (model) {
								auto playerTrans = model->world;
								auto playerTransInve = model->world.Invert();

								auto gtsParentTrans = gtsParent->world;
								auto cameraRootTrans = cameraRoot->local;
								// Get Camera Location without alterations from gtsRoot;
								auto cameraLocation = gtsParentTrans*(cameraRootTrans*NiPoint3(0.0,0.0,0.0));
								log::info("cameraLocation: {}", Vector2Str(cameraLocation));

								// Get target location
								auto targetLocationWorld = playerTrans*((playerTransInve*cameraLocation) * scale);

								NiTransform parentTransInve = gtsParent->world.Invert();
								NiTransform cameraRootTransInve = cameraRootTrans.Invert();
								auto targetLocationLocal = parentTransInve * (cameraRootTransInve * targetLocationWorld);

								// Add adjustments
								// log::info("Delta: {},{}", this->deltaX, this->deltaZ);
								// targetLocationLocal.x += this->deltaX * scale;
								// targetLocationLocal.z += this->deltaZ * scale ;

								// Set Camera
								gtsRoot->local.translate = targetLocationLocal;
								update_node(gtsRoot);
							}
						}
					}
				}
			}
		}
	}

	void CameraManager::AdjustUpDown(float amt) {
		this->deltaZ += amt;
	}
	void CameraManager::ResetUpDown() {
		this->deltaZ = 0.0;
	}

	void CameraManager::AdjustLeftRight(float amt) {
		this->deltaX += amt;
	}
	void CameraManager::ResetLeftRight() {
		this->deltaX = 0.0;
	}
}
