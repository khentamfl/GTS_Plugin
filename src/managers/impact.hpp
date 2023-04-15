#pragma once
// Module that handles footsteps


using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	class ImpactManager : public EventListener {
		public:
			[[nodiscard]] static ImpactManager& GetSingleton() noexcept;

			void HookProcessEvent(BGSImpactManager* impact, const BGSFootstepEvent* a_event, BSTEventSource<BGSFootstepEvent>* a_eventSource);
	};
}
