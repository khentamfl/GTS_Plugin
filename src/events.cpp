#include "events.hpp"

using namespace std;
using namespace RE;
using namespace SKSE;

namespace Gts {
	// Called on Live (non paused) gameplay
	void EventListener::Update() {
	}

	// Called on Papyrus OnUpdate
	void EventListener::PapyrusUpdate() {

	}

	// Called on Havok update (when processing hitjobs)
	void EventListener::HavokUpdate() {

	}

	// Called on game load started (not yet finished)
	// and when new game is selected
	void EventListener::Reset() {
	}

	// Called when game is enabled (while not paused)
	void EventListener::Enabled() {
	}

	// Called when game is disabled (while not paused)
	void EventListener::Disabled() {
	}

	// Called when a game is started after a load/newgame
	void EventListener::Start() {
	}

	// Called when all forms are loaded (during game load before mainmenu)
	void EventListener::DataReady() {

	}

	// Called when all forms are loaded (during game load before mainmenu)
	void EventListener::ResetActor(Actor* actor) {

	}

	// Called when an actor has an item equipped
	void EventListener::ActorEquip(Actor* actor) {

	}

	// Called when an actor has is fully loaded
	void EventListener::ActorLoaded(Actor* actor) {

	}

	// Called when a papyrus hit event is fired
	void EventListener::HitEvent(const TESHitEvent* evt) {
	}

	void EventDispatcher::ReportProfilers() {
		log::info("Reporting Profilers:");
		for (auto listener: EventDispatcher::GetSingleton().listeners) {
			log::info("  - {}: {:.3f}s", listener->DebugName(), listener->profiler.Elapsed());
		}
	}

	void EventDispatcher::AddListener(EventListener* listener) {
		if (listener) {
			EventDispatcher::GetSingleton().listeners.push_back(listener);
		}
	}

	void EventDispatcher::DoUpdate() {
		for (auto listener: EventDispatcher::GetSingleton().listeners) {
			listener->profiler.Start();
			listener->Update();
			listener->profiler.Stop();
		}
	}
	void EventDispatcher::DoPapyrusUpdate() {
		for (auto listener: EventDispatcher::GetSingleton().listeners) {
			listener->profiler.Start();
			listener->PapyrusUpdate();
			listener->profiler.Stop();
		}
	}
	void EventDispatcher::DoHavokUpdate() {
		for (auto listener: EventDispatcher::GetSingleton().listeners) {
			listener->profiler.Start();
			listener->HavokUpdate();
			listener->profiler.Stop();
		}
	}
	void EventDispatcher::DoReset() {
		for (auto listener: EventDispatcher::GetSingleton().listeners) {
			listener->profiler.Start();
			listener->Reset();
			listener->profiler.Stop();
		}
	}
	void EventDispatcher::DoEnabled() {
		for (auto listener: EventDispatcher::GetSingleton().listeners) {
			listener->profiler.Start();
			listener->Enabled();
			listener->profiler.Stop();
		}
	}
	void EventDispatcher::DoDisabled() {
		for (auto listener: EventDispatcher::GetSingleton().listeners) {
			listener->profiler.Start();
			listener->Disabled();
			listener->profiler.Stop();
		}
	}
	void EventDispatcher::DoStart() {
		for (auto listener: EventDispatcher::GetSingleton().listeners) {
			listener->profiler.Start();
			listener->Start();
			listener->profiler.Stop();
		}
	}
	void EventDispatcher::DoDataReady() {
		for (auto listener: EventDispatcher::GetSingleton().listeners) {
			listener->profiler.Start();
			listener->DataReady();
			listener->profiler.Stop();
		}
	}
	void EventDispatcher::DoResetActor(Actor* actor) {
		for (auto listener: EventDispatcher::GetSingleton().listeners) {
			listener->profiler.Start();
			listener->ResetActor(actor);
			listener->profiler.Stop();
		}
	}
	void EventDispatcher::DoActorEquip(Actor* actor) {
		for (auto listener: EventDispatcher::GetSingleton().listeners) {
			listener->profiler.Start();
			listener->ActorEquip(actor);
			listener->profiler.Stop();
		}
	}
	void EventDispatcher::DoActorLoaded(Actor* actor) {
		for (auto listener: EventDispatcher::GetSingleton().listeners) {
			listener->profiler.Start();
			listener->ActorLoaded(actor);
			listener->profiler.Stop();
		}
	}
	void EventDispatcher::DoHitEvent(const TESHitEvent* evt) {
		for (auto listener: EventDispatcher::GetSingleton().listeners) {
			listener->profiler.Start();
			listener->HitEvent(evt);
			listener->profiler.Stop();
		}
	}
	EventDispatcher& EventDispatcher::GetSingleton() {
		static EventDispatcher instance;
		return instance;
	}
}
