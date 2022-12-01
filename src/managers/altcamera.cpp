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

// #define ENABLED_SHADOW

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

	NiCamera* GetNiCamera() {
		auto camera = PlayerCamera::GetSingleton();
		auto cameraRoot = camera->cameraRoot.get();
		NiCamera* niCamera = nullptr;
		for (auto child: cameraRoot->GetChildren()) {
			NiAVObject* node = child.get();
			if (node) {
				NiCamera* casted = netimmerse_cast<NiCamera*>(node);
				if (casted) {
					niCamera = casted;
					break;
				}
			}
		}
		return niCamera;
	}
	void UpdateWorld2ScreetMat(NiCamera* niCamera) {
		auto camNi = niCamera ? niCamera : GetNiCamera();
		typedef void (*UpdateWorldToScreenMtx)(RE::NiCamera*);
		static auto toScreenFunc = REL::Relocation<UpdateWorldToScreenMtx>(REL::RelocationID(69271, 70641).address());
		toScreenFunc(camNi);
	}

	#ifdef ENABLED_SHADOW
	ShadowSceneNode* GetShadowMap() {
		auto player = PlayerCharacter::GetSingleton();
		if (player) {
			auto searchRoot = player->GetCurrent3D();
			if (searchRoot) {
				NiNode* parent = searchRoot->parent;
				while (parent) {
					log::info("- {}", GetRawName(parent));
					log::info("- {}", parent->name);
					ShadowSceneNode* shadowNode = skyrim_cast<ShadowSceneNode*>(parent);
					if (shadowNode) {
						return shadowNode;
					}
					parent = parent->parent;
				}
			}
		}
		return nullptr;
	}
	#endif

	void UpdateSceneManager(NiPoint3 camLoc) {
		auto sceneManager = UI3DSceneManager::GetSingleton();
		if (sceneManager) {
			log::info("Update sceneManager");
			// Cache
			sceneManager->cachedCameraPos = camLoc;

			#ifdef ENABLED_SHADOW
			// Shadow Map
			auto shadowNode = sceneManager->shadowSceneNode;
			if (shadowNode) {
				shadowNode->GetRuntimeData().cameraPos = camLoc;
			}
			#endif

			// Camera
			auto niCamera = sceneManager->camera;
			if (niCamera) {
				niCamera->world.translate = camLoc;
				UpdateWorld2ScreetMat(niCamera.get());
			}
		}
	}

	void UpdateRenderManager(NiPoint3 camLoc) {
		auto renderManager = UIRenderManager::GetSingleton();
		if (renderManager) {
			log::info("Update renderManager");

			#ifdef ENABLED_SHADOW
			// Shadow Map
			auto shadowNode = renderManager->shadowSceneNode;
			if (shadowNode) {
				shadowNode->GetRuntimeData().cameraPos = camLoc;
			}
			#endif

			// Camera
			auto niCamera = renderManager->camera;
			if (niCamera) {
				niCamera->world.translate = camLoc;
				UpdateWorld2ScreetMat(niCamera.get());
			}
		}
	}

	void UpdateNiCamera(NiPoint3 camLoc) {
		auto niCamera = GetNiCamera();
		if (niCamera) {
			niCamera->world.translate = camLoc;
			UpdateWorld2ScreetMat(niCamera);
			update_node(niCamera);
		}

		#ifdef ENABLED_SHADOW
		auto shadowNode = GetShadowMap();
		if (shadowNode) {
			shadowNode->GetRuntimeData().cameraPos = camLoc;
		}
		#endif
	}

	void UpdatePlayerCamera(NiPoint3 camLoc) {
		auto camera = PlayerCamera::GetSingleton();
		if (camera) {
			auto cameraRoot = camera->cameraRoot;
			if (cameraRoot) {
				cameraRoot->local.translate = camLoc;
				cameraRoot->world.translate = camLoc;
				update_node(cameraRoot.get());
			}
		}
	}

	void ScaleCamera(NiPoint3 offset) {
		auto camera = PlayerCamera::GetSingleton();
		auto cameraRoot = camera->cameraRoot;
		auto player = PlayerCharacter::GetSingleton();
		auto currentState = camera->currentState;

		if (cameraRoot) {
			NiPoint3 cameraLocation;
			currentState->GetTranslation(cameraLocation);
			if (currentState) {
				if (player) {
					float scale = get_visual_scale(player);
					if (scale > 1e-4) {
						auto model = player->Get3D(false);
						if (model) {
							auto playerTrans = model->world;
							auto playerTransInve = model->world.Invert();

							// Get Scaled Camera Location
							auto targetLocationWorld = playerTrans*((playerTransInve*cameraLocation) * scale);
							auto parent = cameraRoot->parent;
							NiTransform transform = parent->world.Invert();
							auto targetLocationLocal = transform * targetLocationWorld;

							// Add adjustments
							targetLocationLocal.x += offset.x * scale;
							targetLocationLocal.z += offset.z * scale;

							UpdatePlayerCamera(targetLocationLocal);
							UpdateNiCamera(targetLocationLocal);
							UpdateSceneManager(targetLocationLocal);
							UpdateRenderManager(targetLocationLocal);
						}
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
		ResetIniSettings();
	}

	void CameraManager::UpdateCamera() {
		CameraManager::GetSingleton().ApplyCamera();
	}

	void CameraManager::ApplyCamera() {
		NiPoint3 offset;
		offset.x += this->deltaX;
		offset.z += this->deltaZ;
		ScaleCamera(offset);
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
