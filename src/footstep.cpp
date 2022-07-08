#include "footstep.h"
#include "scale.h"

using namespace SKSE;
using namespace RE;

namespace Gts {
	FootStepManager& FootStepManager::GetSingleton() noexcept {
		static FootStepManager instance;
		return instance;
	}

	void FootStepManager::HookProcessEvent(const BGSFootstepEvent* a_event, BSTEventSource<BGSFootstepEvent>* a_eventSource) {
		log::info("+ Foot step event Hook");
		if (a_event) {
			auto actor = a_event->actor.get().get();
			if (actor) {
				auto base_actor = actor->GetActorBase();
				log::info("  - Event for: {}", base_actor->GetFullName());
			}
			log::info("  - Tag: {}", a_event->tag);
		} else {
			log::info("  - No event data supplied");
		}
		log::info("- Foot step event Hook");
	}
}
