
#include "hooks.h"
#include "GtsManager.h"

using namespace RE;

namespace Hooks
{
	void Install()
	{
		logger::info("Gts applying hooks...");

		MainUpdateHook::Hook();
		HookOnPlayerUpdate::Hook();
		HookOnActorUpdate::Hook();
		HookbhkCharProxyController::Hook();

		logger::info("Gts finished applying hooks...");
	}

	void MainUpdateHook::Hook()
	{
		logger::info("Gts applying MainUpdateHook");

		auto& trampoline = SKSE::GetTrampoline();
		trampoline.create(256);

		REL::Relocation<uintptr_t> hook{REL::RelocationID(35551, 36544)};          // main loop
		_Update = trampoline.write_call<5>(hook.address() + REL::Relocate(0x11F, 0x160), Update);
	}

	void MainUpdateHook::Update(RE::Main* a_this, float a2)
	{
		_Update(a_this, a2);

		// logger::info("Update");
	}


	void HookOnPlayerUpdate::Hook() {
		logger::info("Hooking player update");
		REL::Relocation<std::uintptr_t> PlayerCharacterVtbl{ RE::VTABLE_PlayerCharacter[0] };

		_Update = PlayerCharacterVtbl.write_vfunc(0xAD, Update);
	}

	void HookOnPlayerUpdate::Update(RE::PlayerCharacter* a_this, float a_delta) {
		_Update(a_this, a_delta);

		Gts::GtsManager::GetSingleton().poll_actor(a_this);
	}

	void HookOnActorUpdate::Hook() {
		logger::info("Hooking character update");
		REL::Relocation<std::uintptr_t> ActorVtbl{ RE::VTABLE_Character[0] };

		_Update = ActorVtbl.write_vfunc(0xAD, Update);

	}

	void HookOnActorUpdate::Update(RE::Actor* a_this, float a_delta) {
		_Update(a_this, a_delta);

		Gts::GtsManager::GetSingleton().poll_actor(a_this);
	}

	void HookbhkCharProxyController::Hook() {
		logger::info("Hooking character proxy controller");
		REL::Relocation<std::uintptr_t> ActorVtbl{ RE::VTABLE_bhkCharProxyController[0] };

		// CharacterInteractionCallback
		_Orig = ActorVtbl.write_vfunc(0x04, CharacterInteractionCallback);

	}

	void HookbhkCharProxyController::CharacterInteractionCallback(bhkCharProxyController* a_this, hkpCharacterProxy* a_proxy, hkpCharacterProxy* a_otherProxy, const hkContactPoint& a_contact) {
		logger::info("Char char collision");
		_Orig(a_this, a_proxy, a_otherProxy, a_contact);
	}

	void HookhkpCharacterProxyListener::Hook() {
		logger::info("Hooking character proxy listener");
		REL::Relocation<std::uintptr_t> ActorVtbl{ RE::VTABLE_hkpCharacterProxyListener[0] };

		// CharacterInteractionCallback
		_Orig = ActorVtbl.write_vfunc(0x04, CharacterInteractionCallback);

	}

	void HookhkpCharacterProxyListener::CharacterInteractionCallback(hkpCharacterProxyListener* a_this, hkpCharacterProxy* a_proxy, hkpCharacterProxy* a_otherProxy, const hkContactPoint& a_contact) {
		logger::info("Char char collision listener");
		_Orig(a_this, a_proxy, a_otherProxy, a_contact);
	}
}
