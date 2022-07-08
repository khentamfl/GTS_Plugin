#pragma once
// Module that handles footsteps
#include <SKSE/SKSE.h>

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	class FootStepManager : public BSTEventSink<BGSFootstepEvent> {
		public:
			[[nodiscard]] static FootStepManager& GetSingleton() noexcept;

			BSEventNotifyControl ProcessEvent(const BGSFootstepEvent* a_event, BSTEventSource<BGSFootstepEvent>* a_eventSource) override;

			bool RegisterSink();
	};
}
