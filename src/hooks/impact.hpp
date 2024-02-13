#pragma once
#include "hooks/hooks.hpp"

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

			static BSEventNotifyControl ProcessEvent_Coll(BGSImpactManager* a_this, const BGSCollisionSoundEvent* a_event, BSTEventSource<BGSCollisionSoundEvent>* a_eventSource);
			static inline REL::Relocation<decltype(ProcessEvent_Coll)> _ProcessEvent_Coll;
	};
}
