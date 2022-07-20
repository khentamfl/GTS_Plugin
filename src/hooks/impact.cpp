#include "hooks/impact.h"
#include "footstep.h"

using namespace RE;

namespace Hooks
{
	// BGSImpactManager
	void Hook_BGSImpactManager::Hook() {
		logger::info("Hooking BGSImpactManager");
		REL::Relocation<std::uintptr_t> Vtbl{ RE::VTABLE_BGSImpactManager[0] };

		_ProcessEvent = Vtbl.write_vfunc(0x01, ProcessEvent);
	}

	BSEventNotifyControl Hook_BGSImpactManager::ProcessEvent(BGSImpactManager* a_this, const BGSFootstepEvent* a_event, BSTEventSource<BGSFootstepEvent>* a_eventSource) {
		Gts::FootStepManager::GetSingleton().HookProcessEvent(a_this, a_event, a_eventSource);
		auto result = _ProcessEvent(a_this, a_event, a_eventSource);
		return result;
	}
}
