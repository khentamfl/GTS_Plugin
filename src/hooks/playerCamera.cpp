#include "hooks/playerCamera.hpp"
#include "data/plugin.hpp"
#include "util.hpp"

using namespace RE;
using namespace Gts;

namespace Hooks
{
	// BGSImpactManager
	void Hook_PlayerCamera::Hook() {
		logger::info("Hooking PlayerCamera");
		REL::Relocation<std::uintptr_t> Vtbl{ RE::VTABLE_PlayerCamera[0] };
		// _SetCameraRoot = Vtbl.write_vfunc(0x01, SetCameraRoot);
		_Update = Vtbl.write_vfunc(0x02, Update);
	}

	void Hook_PlayerCamera::SetCameraRoot(TESCamera* a_this, NiNode* a_root) {
		log::info("Player camera SetCameraRoot");
		log::info("Grabbing RAW NAME");
		log::info("a_this: {}", Gts::GetRawName(a_this));
		log::info("a_root: {}", Gts::GetRawName(a_root));
		if (Gts::Plugin::InGame()) {
			log::info("In Game");
			auto player = PlayerCharacter::GetSingleton();
			if (player) {
				log::info("Player exists");
				if (a_root) {
					log::info("Getting name");
					log::trace("  - Node {}", a_root->name);
				}
				log::info("Getting model");
				auto model = player->GetCurrent3D();
				if (model) {
					auto node = model->AsNode();
					if (node) {
						log::info("Alternative Set");
						_SetCameraRoot(a_this, node);
						return;
					}
				}
			}
		}
		log::info("Normal Set");
		_SetCameraRoot(a_this, a_root);
	}

	void Hook_PlayerCamera::Update(TESCamera* a_this) {
		auto camera = PlayerCamera::GetSingleton();
		auto third = skyrim_cast<ThirdPersonState*>(camera->cameraStates[CameraState::kThirdPerson].get());
		log::info("Pre PlayerCamera Update");
		log::info("cam node pos: {}::{}", Vector2Str(third->thirdPersonCameraObj->world.translate), Vector2Str(third->thirdPersonCameraObj->local.translate));
		_Update(a_this);
		log::info("Post PlayerCamera Update");
		log::info("cam node pos: {}::{}", Vector2Str(third->thirdPersonCameraObj->world.translate), Vector2Str(third->thirdPersonCameraObj->local.translate));

	}
}
