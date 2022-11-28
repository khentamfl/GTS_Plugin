#include "hooks/cameraState.hpp"
#include "util.hpp"
#include "scale/scale.hpp"
#include "node.hpp"

using namespace RE;
using namespace Gts;

namespace Hooks
{
	void Hook_CameraState::Hook() {
		logger::info("Hooking ThirdPersonState");
		REL::Relocation<std::uintptr_t> Vtbl{ ThirdPersonState::VTABLE[0] };
		_Update = Vtbl.write_vfunc(0x03, Update);
		// _GetRotation = Vtbl.write_vfunc(0x04, GetRotation);
		// _GetTranslation = Vtbl.write_vfunc(0x05, GetTranslation);
	}

	void Hook_CameraState::Update(ThirdPersonState* a_this, BSTSmartPointer<TESCameraState>& a_nextState) {
		log::info("Update: {}", GetRawName(a_this));
		if (a_nextState) {
			log::info("  - {}", GetRawName(a_nextState.get()));
		}

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
									log::info("cameraLocation: {}", Vector2Str(cameraLocation));
									auto targetLocationWorld = playerTrans*((playerTransInve*cameraLocation) * scale);
									auto parent = cameraRoot->parent;
									NiTransform transform = parent->world.Invert();
									auto targetLocationLocal = transform * targetLocationWorld;
									log::info("targetLocationLocal: {}", Vector2Str(targetLocationLocal));

									// Set Camera
									cameraRoot->local.translate = targetLocationLocal;
									a_this->translation = targetLocationLocal;
									update_node(cameraRoot.get());
								}
							}
						}
					}
				}
			}
		}

		_Update(a_this, a_nextState);
	}

	void Hook_CameraState::GetRotation(ThirdPersonState* a_this, NiQuaternion& a_rotation) {
		log::info("GetRotation: {}", Vector2Str(a_rotation));
		_GetRotation(a_this, a_rotation);
	}

	void Hook_CameraState::GetTranslation(ThirdPersonState* a_this, NiPoint3& a_translation) {
		log::info("GetTranslation", Vector2Str(a_translation));
		_GetTranslation(a_this, a_translation);
	}
}
