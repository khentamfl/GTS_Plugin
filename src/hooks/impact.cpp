#include "hooks/impact.h"
#include "managers/footstep.h"

using namespace RE;

namespace Hooks
{
	// BGSImpactManager
	void Hook_BGSImpactManager::Hook() {
		logger::info("Hooking BGSImpactManager");
		REL::Relocation<std::uintptr_t> Vtbl{ RE::VTABLE_BGSImpactManager[0] };
		_ProcessEvent = Vtbl.write_vfunc(0x01, ProcessEvent);

		REL::Relocation<std::uintptr_t> Vtbl1{ RE::VTABLE_BGSImpactManager[1] };
		_ProcessEventCombat = Vtbl1.write_vfunc(0x01, ProcessEventCombat);

		REL::Relocation<std::uintptr_t> Vtbl2{ RE::VTABLE_BGSImpactManager[2] };
		_ProcessEventCollision = Vtbl2.write_vfunc(0x01, ProcessEventCollision);
	}

	BSEventNotifyControl Hook_BGSImpactManager::ProcessEvent(BGSImpactManager* a_this, const BGSFootstepEvent* a_event, BSTEventSource<BGSFootstepEvent>* a_eventSource) {
		Gts::FootStepManager::GetSingleton().HookProcessEvent(a_this, a_event, a_eventSource);
		auto result = _ProcessEvent(a_this, a_event, a_eventSource);
		return result;
	}

	BSEventNotifyControl Hook_BGSImpactManager::ProcessEventCombat(BGSImpactManager* a_this, const BGSCombatImpactEvent* a_event, BSTEventSource<BGSCombatImpactEvent>* a_eventSource) {
		logger::info("Combat Impact");
		auto result = _ProcessEventCombat(a_this, a_event, a_eventSource);
		return result;
	}

	BSEventNotifyControl Hook_BGSImpactManager::ProcessEventCollision(BGSImpactManager* a_this, const BGSCollisionSoundEvent* a_event, BSTEventSource<BGSCollisionSoundEvent>* a_eventSource) {
		logger::info("Collision Impact");
		auto result = _ProcessEventCollision(a_this, a_event, a_eventSource);
		return result;
	}
}
