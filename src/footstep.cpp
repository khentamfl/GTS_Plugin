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
		auto actor = a_event->actor.get().get();
		if (actor) {
			auto base_actor = actor->GetActorBase();
			log::info("  - Event for: {}", base_actor->GetFullName());
		}
		log::info("  - Tag: {}", a_event->tag);
		log::info("- Foot step event Hook");
	}

	bool FootStepManager::RegisterSink() {
		auto manager = BGSFootstepManager::GetSingleton();
		if (!manager) {
			return false;
		}
		manager->AddEventSink(this);
		return true;
	}

	BSEventNotifyControl FootStepManager::ProcessEvent(const BGSFootstepEvent* a_event, BSTEventSource<BGSFootstepEvent>* a_eventSource) {
		log::info("+ Foot step event Skyrim");
		auto actor = a_event->actor.get().get();
		if (actor) {
			auto base_actor = actor->GetActorBase();
			log::info("  - Event for: {}", base_actor->GetFullName());
		}
		log::info("  - Tag: {}", a_event->tag);
		log::info("- Foot step event Skyrim");

		return BSEventNotifyControl::kContinue;
	}
}
