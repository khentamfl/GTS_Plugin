#include "reloader.h"
#include "GtsManager.h"

using namespace SKSE;
using namespace RE;

namespace Gts {
	ReloadManager& ReloadManager::GetSingleton() noexcept {
		static ReloadManager instance;
		return instance;
	}

	BSEventNotifyControl ReloadManager::ProcessEvent(const TESObjectLoadedEvent * evn, BSTEventSource<TESObjectLoadedEvent>* dispatcher)
	{
		if (evn) {
			auto* actor = TESForm::LookupByID<Actor>(evn->formID);
			if (actor) {
				log::info("Got actor");
				GtsManager::GetSingleton().reapply_actor(actor);
			}
		}
		return BSEventNotifyControl::kContinue;
	}

	BSEventNotifyControl ReloadManager::ProcessEvent(const TESCellFullyLoadedEvent* evn, BSTEventSource<TESCellFullyLoadedEvent>* dispatcher)
	{
		log::info("Cell fully loaded");
		GtsManager::GetSingleton().reapply();
		return BSEventNotifyControl::kContinue;
	}
}
