#pragma once
using namespace RE;
using namespace SKSE;

namespace Hooks
{
	class Hook_BGSImpactManager
	{
		public:
			static void Hook();
		private:

			static BSEventNotifyControl ProcessEvent(BGSImpactManager* a_this, const BGSFootstepEvent* a_event, BSTEventSource<BGSFootstepEvent>* a_eventSource);
			static inline REL::Relocation<decltype(ProcessEvent)> _ProcessEvent;

			static BSEventNotifyControl ProcessEventCombat(BGSImpactManager* a_this, const BGSCombatImpactEvent* a_event, BSTEventSource<BGSCombatImpactEvent>* a_eventSource);
			static inline REL::Relocation<decltype(ProcessEventCombat)> _ProcessEventCombat;

			static BSEventNotifyControl ProcessEventCollision(BGSImpactManager* a_this, const BGSCollisionSoundEvent* a_event, BSTEventSource<BGSCollisionSoundEvent>* a_eventSource);
			static inline REL::Relocation<decltype(ProcessEventCollision)> _ProcessEventCollision;
	};
}
