
#include "hooks.h"
#include "GtsManager.h"


namespace Hooks
{
	void Install()
	{
		logger::info("Gts applying hooks...");

		MainUpdateHook::Hook();
		HookOnPlayerUpdate::Hook();

		logger::info("Gts finished applying hooks...");
	}

	void MainUpdateHook::Hook()
	{
		logger::info("Gts applying MainUpdateHook");

		auto& trampoline = SKSE::GetTrampoline();

		REL::Relocation<uintptr_t> hook(REL::RelocationID(35551, 36544));          // main loop
		_Update = trampoline.write_call<5>(hook.address() + REL::Relocate(0x11F, 0x160), Update);
	}

	void MainUpdateHook::Update(RE::Main* a_this, float a2)
	{
		_Update(a_this, a2);

		logger::info("Update");
	}


	void HookOnPlayerUpdate::Hook() {
		logger::info("Hooking player update");
		REL::Relocation<std::uintptr_t> PlayerCharacterVtbl{ RE::VTABLE_PlayerCharacter[0] };

		_Update = PlayerCharacterVtbl.write_vfunc(0xAD, Update);

	}

	void HookOnPlayerUpdate::Update(RE::PlayerCharacter* a_this, float a_delta) {
		_Update(a_this, a_delta);

		auto base_actor = a_this->GetActorBase();
		auto actor_name = base_actor->GetFullName();
		logger::info("Update player: {}", actor_name);
	}

	void HookOnActorUpdate::Hook() {
		logger::info("Hooking character update");
		REL::Relocation<std::uintptr_t> ActorVtbl{ RE::VTABLE_Character[0] };

		_Update = ActorVtbl.write_vfunc(0xAD, Update);

	}

	void HookOnActorUpdate::Update(RE::Actor* a_this, float a_delta) {
		_Update(a_this, a_delta);

		auto base_actor = a_this->GetActorBase();
		auto actor_name = base_actor->GetFullName();
		logger::info("Update actor: {}", actor_name);
	}
}
