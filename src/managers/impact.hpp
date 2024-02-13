#pragma once
#include "events.hpp"
// Module that handles footsteps


using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	class ImpactManager {
		public:
			[[nodiscard]] static ImpactManager& GetSingleton() noexcept;

			void HookCollisionEvent(BGSImpactManager* impact, const BGSCollisionSoundEvent* a_event, BSTEventSource<BGSCollisionSoundEvent>* a_eventSource);
			void HookProcessEvent(BGSImpactManager* impact, const BGSFootstepEvent* a_event, BSTEventSource<BGSFootstepEvent>* a_eventSource);
	};
}
