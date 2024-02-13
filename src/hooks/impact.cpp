#include "hooks/impact.hpp"
#include "managers/impact.hpp"

using namespace RE;

namespace Hooks
{
	// BGSImpactManager
	void Hook_BGSImpactManager::Hook() {
		log::info("Hooking BGSImpactManager");
		REL::Relocation<std::uintptr_t> Vtbl{ RE::VTABLE_BGSImpactManager[0] };
		_ProcessEvent = Vtbl.write_vfunc(0x01, ProcessEvent);

		REL::Relocation<std::uintptr_t> Vtbl_1{ RE::VTABLE_BGSImpactManager[1] };
		_ProcessEvent_Coll = Vtbl_1.write_vfunc(0x01, ProcessEvent_Coll);
	}

	BSEventNotifyControl Hook_BGSImpactManager::ProcessEvent(BGSImpactManager* a_this, const BGSFootstepEvent* a_event, BSTEventSource<BGSFootstepEvent>* a_eventSource) {
		Gts::ImpactManager::GetSingleton().HookProcessEvent(a_this, a_event, a_eventSource);
		auto result = _ProcessEvent(a_this, a_event, a_eventSource);
		return result;
	}

	BSEventNotifyControl Hook_BGSImpactManager::ProcessEvent_Coll(BGSImpactManager* a_this, const BGSCollisionSoundEvent* a_event, BSTEventSource<BGSCollisionSoundEvent>* a_eventSource) {
		Gts::ImpactManager::GetSingleton().HookCollisionEvent(a_this, a_event, a_eventSource);
		auto result = _ProcessEvent_Coll(a_this, a_event, a_eventSource);
		return result;
	}
}
