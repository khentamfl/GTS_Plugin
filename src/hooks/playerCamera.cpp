#include "hooks/playerCamera.hpp"

using namespace RE;

namespace Hooks
{
	// BGSImpactManager
	void Hook_PlayerCamera::Hook() {
		logger::info("Hooking PlayerCamera");
		REL::Relocation<std::uintptr_t> Vtbl{ RE::VTABLE_PlayerCamera[0] };
		_SetCameraRoot = Vtbl.write_vfunc(0x01, SetCameraRoot);
		_Update = Vtbl.write_vfunc(0x02, Update);
	}

	void Hook_PlayerCamera::SetCameraRoot(PlayerCamera* a_this, NiPointer<NiNode> a_root) {
		log::info("Player camera SetCameraRoot");
		if (a_root) {
			log::trace("  - Node {}", a_root->name);
		}
		auto player = PlayerCharacter::GetSingleton();
		if (player) {
			auto model = player->GetCurrent3D();
			if (model) {
				_SetCameraRoot(a_this, NiPointer<NiNode>(model));
				return;
			}
		}
		_SetCameraRoot(a_this, nullptr);
	}

	void Hook_PlayerCamera::Update(PlayerCamera* a_this) {
		log::info("Player camera Update");
		_Update(a_this);
	}
}
