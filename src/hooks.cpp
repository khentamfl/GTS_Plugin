
#include "hooks.h"
#include "GtsManager.h"


namespace Hooks
{
	void Install()
	{
		logger::info("Gts applying hooks...");

		MainUpdateHook::Hook();

		logger::info("Gts finished applying hooks...");
	}

	void MainUpdateHook::Hook()
	{
		logger::info("Gts applying MainUpdateHook");

		auto& trampoline = SKSE::GetTrampoline();
		trampoline.create(64);

		REL::Relocation<uintptr_t> hook(REL::RelocationID(35551, 36544), REL::Relocate(0x11F, 0x160));          // main loop
		_Update = trampoline.write_call<5>(hook.address(), Update);
	}

	void MainUpdateHook::Update(RE::Main* a_this, float a2)
	{
		_Update(a_this, a2);

		Gts::GtsManager::GetSingleton().poll();
	}



}
