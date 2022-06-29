
#include "hooks.h"
#include "GtsManager.h"

using namespace RE;

namespace Hooks
{
	void Install()
	{
		logger::info("Gts applying hooks...");

		auto& trampoline = SKSE::GetTrampoline();
		trampoline.create(256);

		Hook_MainUpdate::Hook(trampoline);
		Hook_OnPlayerUpdate::Hook();
		Hook_OnActorUpdate::Hook();
		Hook_bhkCharProxyController::Hook();
		Hook_hkpCharacterProxyListener::Hook();

		logger::info("Gts finished applying hooks...");
	}

	// Main
	void Hook_MainUpdate::Hook(Trampoline& trampoline)
	{
		REL::Relocation<uintptr_t> hook{REL::RelocationID(35551, 36544)};          // main loop
		logger::info("Gts applying Main Update Hook at {}", hook.address());
		_Update = trampoline.write_call<5>(hook.address() + REL::Relocate(0x11F, 0x160), Update);
	}

	void Hook_MainUpdate::Update(RE::Main* a_this, float a2)
	{
		_Update(a_this, a2);
		Gts::GtsManager::GetSingleton().poll();
	}

	// PLayer update
	void Hook_OnPlayerUpdate::Hook() {
		logger::info("Hooking player update");
		REL::Relocation<std::uintptr_t> PlayerCharacterVtbl{ RE::VTABLE_PlayerCharacter[0] };

		_Update = PlayerCharacterVtbl.write_vfunc(0xAD, Update);
	}

	void Hook_OnPlayerUpdate::Update(RE::PlayerCharacter* a_this, float a_delta) {
		_Update(a_this, a_delta);

		//Gts::GtsManager::GetSingleton().poll_actor(a_this);
	}

	// Actor update
	void Hook_OnActorUpdate::Hook() {
		logger::info("Hooking character update");
		REL::Relocation<std::uintptr_t> ActorVtbl{ RE::VTABLE_Character[0] };

		_Update = ActorVtbl.write_vfunc(0xAD, Update);

	}

	void Hook_OnActorUpdate::Update(RE::Actor* a_this, float a_delta) {
		_Update(a_this, a_delta);

		Gts::GtsManager::GetSingleton().poll_actor(a_this);
	}

	// bhkCharProxyController
	void Hook_bhkCharProxyController::Hook() {
		logger::info("Hooking character proxy controller");
		REL::Relocation<std::uintptr_t> ActorVtbl{ RE::VTABLE_bhkCharProxyController[0] };

		// CharacterInteractionCallback
		_Orig = ActorVtbl.write_vfunc(0x04, CharacterInteractionCallback);

	}

	void Hook_bhkCharProxyController::CharacterInteractionCallback(bhkCharProxyController* a_this, hkpCharacterProxy* a_proxy, hkpCharacterProxy* a_otherProxy, const hkContactPoint& a_contact) {
		logger::info("Char char collision");
		_Orig(a_this, a_proxy, a_otherProxy, a_contact);
	}

	// hkpCharacterProxyListener
	void Hook_hkpCharacterProxyListener::Hook() {
		logger::info("Hooking character proxy listener");
		REL::Relocation<std::uintptr_t> ActorVtbl{ RE::VTABLE_hkpCharacterProxyListener[0] };

		// CharacterInteractionCallback
		_Orig = ActorVtbl.write_vfunc(0x04, CharacterInteractionCallback);

	}

	void Hook_hkpCharacterProxyListener::CharacterInteractionCallback(hkpCharacterProxyListener* a_this, hkpCharacterProxy* a_proxy, hkpCharacterProxy* a_otherProxy, const hkContactPoint& a_contact) {
		logger::info("Char char collision listener");
		_Orig(a_this, a_proxy, a_otherProxy, a_contact);
	}

}
