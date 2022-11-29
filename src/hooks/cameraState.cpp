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
	}
	void UpdateWorld2ScreetMat(NiCamera* nicamera) {
		auto camNi = niCamera ? niCamera : GetNiCamera();
		typedef void (*UpdateWorldToScreenMtx)(RE::NiCamera*);
		static auto toScreenFunc = REL::Relocation<UpdateWorldToScreenMtx>(REL::RelocationID(69271, 70641).address());
		toScreenFunc(camNi);
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
			// When a_nextState is nullptr we are updating current state
			auto camera = PlayerCamera::GetSingleton();
			if (camera) {
				if (camera == a_this->camera) {
					auto cameraRoot = camera->cameraRoot;
					if (cameraRoot) {
						NiPoint3 thirdLocation;
						a_this->GetTranslation(thirdLocation);

						auto player = PlayerCharacter::GetSingleton();
						if (player) {
							float scale = get_visual_scale(player);
							if (scale > 1e-4) {
								auto model = player->Get3D(false);
								if (model) {
									auto playerTrans = model->world;
									auto playerTransInve = model->world.Invert();

									// Get Scaled Camera Location
									auto cameraLocation = thirdLocation;
									auto targetLocationWorld = playerTrans*((playerTransInve*cameraLocation) * scale);
									auto parent = cameraRoot->parent;
									NiTransform transform = parent->world.Invert();
									auto targetLocationLocal = transform * targetLocationWorld;

									// Set Camera
									log::info("PRE: translation: {}", Vector2Str(a_this->translation));
									cameraRoot->local.translate = targetLocationLocal;
									a_this->translation = targetLocationLocal;
									log::info("POST: translation: {}", Vector2Str(a_this->translation));
									update_node(cameraRoot.get());
									UpdateWorld2ScreetMat(nullptr);
								}
							}
						}
					}
				}
			}
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

		auto camera = PlayerCamera::GetSingleton();
		if (camera) {
			if (camera == a_this->camera) {
				auto cameraRoot = camera->cameraRoot;
				if (cameraRoot) {
					NiPoint3 thirdLocation;
					a_this->GetTranslation(thirdLocation);

					auto player = PlayerCharacter::GetSingleton();
					if (player) {
						float scale = get_visual_scale(player);
						if (scale > 1e-4) {
							auto model = player->Get3D(false);
							if (model) {
								auto playerTrans = model->world;
								auto playerTransInve = model->world.Invert();

								// Get Scaled Camera Location
								auto cameraLocation = thirdLocation;
								auto targetLocationWorld = playerTrans*((playerTransInve*cameraLocation) * scale);
								auto parent = cameraRoot->parent;
								NiTransform transform = parent->world.Invert();
								auto targetLocationLocal = transform * targetLocationWorld;

								// Set Camera
								log::info("PRE: translation: {}", Vector2Str(a_this->translation));
								cameraRoot->local.translate = targetLocationLocal;
								a_this->translation = targetLocationLocal;
								log::info("POST: translation: {}", Vector2Str(a_this->translation));
								update_node(cameraRoot.get());
							}
						}
					}
				}
			}
		}
	}
}
