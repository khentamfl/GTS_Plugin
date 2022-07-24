#include "hooks/main.h"

#include "managers/GtsManager.h"

using namespace RE;
using namespace SKSE;

namespace Hooks
{
	void Hook_MainUpdate::Hook(Trampoline& trampoline)
	{
		REL::Relocation<uintptr_t> hook{REL::RelocationID(35551, 36544)};
		logger::info("Gts applying Main Update Hook at {}", hook.address());
		_Update = trampoline.write_call<5>(hook.address() + REL::Relocate(0x11F, 0x160), Update);
	}

	void Hook_MainUpdate::Update(RE::Main* a_this, float a2)
	{
		_Update(a_this, a2);
        auto manager = Gts::GtsManager::GetSingleton();
        manager.main_thread = true;
        manager.poll();
        manager.main_thread = false;
	}
}
