#include "reloader.h"
#include "GtsManager.h"

using namespace SKSE;
using namespace RE;

namespace Gts {
	void ReloadManager::Initialize() {
		auto event_sources = ScriptEventSourceHolder::GetSingleton();
		if (event_sources) {
			event_sources->AddEventSink<TESObjectLoadedEvent>(this);
			event_sources->AddEventSink<TESCellFullyLoadedEvent>(this);
			event_sources->AddEventSink<TESCellAttachDetachEvent>(this);
			event_sources->AddEventSink<TESEquipEvent>(this);
		}
	}
	ReloadManager& ReloadManager::GetSingleton() noexcept {
		static ReloadManager instance;
		return instance;
	}

	BSEventNotifyControl ReloadManager::ProcessEvent(const TESObjectLoadedEvent * evn, BSTEventSource<TESObjectLoadedEvent>* dispatcher)
	{
		if (evn) {
			auto* actor = TESForm::LookupByID<Actor>(evn->formID);
			if (actor) {
				GtsManager::GetSingleton().reapply_actor(actor);
			}
		}
		return BSEventNotifyControl::kContinue;
	}

	BSEventNotifyControl ReloadManager::ProcessEvent(const TESCellFullyLoadedEvent* evn, BSTEventSource<TESCellFullyLoadedEvent>* dispatcher)
	{
		log::info("TESCellFullyLoadedEvent");
		GtsManager::GetSingleton().reapply();
		return BSEventNotifyControl::kContinue;
	}

	BSEventNotifyControl ReloadManager::ProcessEvent(const TESCellAttachDetachEvent* evn, BSTEventSource<TESCellAttachDetachEvent>* dispatcher)
	{
		log::info("TESCellAttachDetachEvent");
		GtsManager::GetSingleton().reapply();
		return BSEventNotifyControl::kContinue;
	}

	BSEventNotifyControl ReloadManager::ProcessEvent(const TESEquipEvent* evn, BSTEventSource<TESEquipEvent>* dispatcher)
	{
		log::info("TESEquipEvent");
		GtsManager::GetSingleton().reapply();
		return BSEventNotifyControl::kContinue;
	}
}
