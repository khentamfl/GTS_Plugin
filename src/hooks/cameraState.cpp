#include "hooks/cameraState.hpp"
#include "util.hpp"
#include "scale/scale.hpp"
#include "node.hpp"

using namespace RE;
using namespace Gts;

namespace {
	NiCamera* GetNiCamera() {
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
		return niCamera;
	}
	void UpdateWorld2ScreetMat(NiCamera* niCamera) {
		auto camNi = niCamera ? niCamera : GetNiCamera();
		typedef void (*UpdateWorldToScreenMtx)(RE::NiCamera*);
		static auto toScreenFunc = REL::Relocation<UpdateWorldToScreenMtx>(REL::RelocationID(69271, 70641).address());
		toScreenFunc(camNi);
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

						// Get Scaled Camera Location
						auto cameraLocation = cameraRoot->world.translate;
						auto targetLocationWorld = playerTrans*((playerTransInve*cameraLocation) * scale);
						auto parent = niCamera->parent;
						NiTransform transform = parent->world.Invert();
						auto targetLocationLocal = transform * targetLocationWorld;

						// Add adjustments
						// log::info("Delta: {},{}", deltaX, deltaY);
						// targetLocationLocal.x += deltaX * scale;
						// targetLocationLocal.y += deltaY * scale;

						// Set Camera
						niCamera->local.translate = targetLocationLocal;
						update_node(niCamera);
						UpdateWorld2ScreetMat(niCamera);
						auto sky = Sky::GetSingleton();
						if (sky) {
							log::info("Sky: {}", GetRawName(sky));
							auto skyRoot = sky->root;
							if (skyRoot) {
								log::info("SkyRoot: {}", GetRawName(skyRoot.get()));
								log::info("SkyRoot Name: {}", skyRoot->name);
								log::info("SkyRoot Pos: {}", Vector2Str(skyRoot->world.translate));
								auto skyParent = skyRoot->parent;
								if (skyParent) {
									log::info("SkyRoot Parent: {}", skyParent->name);
									auto skyParentParent = skyParent->parent;
									if (skyParentParent) {
										log::info("SkyRoot Parent Paren: {}", skyParentParent->name);
									}
								}

								log::info("cameraRoot Location: {}", Vector2Str(cameraLocation));
							}
						}
					}
				}
			}
		}
	}
}

namespace Hooks
{
	void Hook_CameraState::Hook() {
		logger::info("Hooking ThirdPersonState");
		REL::Relocation<std::uintptr_t> Vtbl{ ThirdPersonState::VTABLE[0] };
		_Update = Vtbl.write_vfunc(0x03, Update);
		// _GetRotation = Vtbl.write_vfunc(0x04, GetRotation);
		// _GetTranslation = Vtbl.write_vfunc(0x05, GetTranslation);
		// _UpdateRotation = Vtbl.write_vfunc(0x0E, UpdateRotation);
	}

	void Hook_CameraState::Update(ThirdPersonState* a_this, BSTSmartPointer<TESCameraState>& a_nextState) {
		if (a_nextState) {
			log::info("  - {}", GetRawName(a_nextState.get()));
		}

		_Update(a_this, a_nextState);

		if (!a_nextState) {
			// Experiment09();
		}
	}

	void Hook_CameraState::GetRotation(ThirdPersonState* a_this, NiQuaternion& a_rotation) {
		log::info("GetRotation: {}", Vector2Str(a_rotation));
		_GetRotation(a_this, a_rotation);
	}

	void Hook_CameraState::GetTranslation(ThirdPersonState* a_this, NiPoint3& a_translation) {
		log::info("GetTranslation", Vector2Str(a_translation));
		_GetTranslation(a_this, a_translation);
	}

	void Hook_CameraState::UpdateRotation(ThirdPersonState* a_this) {
		log::info("UpdateRotation");
		_UpdateRotation(a_this);
	}
}
