#include "managers/reloader.hpp"
#include "managers/GtsManager.hpp"
#include "managers/collider.hpp"
#include "UI/DebugAPI.hpp"

using namespace SKSE;
using namespace RE;

namespace Gts {
	void ReloadManager::Initialize() {
		auto event_sources = ScriptEventSourceHolder::GetSingleton();
		if (event_sources) {
			event_sources->AddEventSink<TESObjectLoadedEvent>(this);
			// event_sources->AddEventSink<TESCellFullyLoadedEvent>(this);
			// event_sources->AddEventSink<TESCellAttachDetachEvent>(this);
			event_sources->AddEventSink<TESEquipEvent>(this);
		}

		auto ui = UI::GetSingleton();
		if (ui) {
			ui->AddEventSink<MenuOpenCloseEvent>(this);
			logger::info("Gts: successfully registered MenuOpenCloseEventHandler");
		} else {
			logger::error("Gts: failed to register MenuOpenCloseEventHandler");
			return;
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
				ColliderManager::GetSingleton().ResetActor(actor);
			}
		}
		return BSEventNotifyControl::kContinue;
	}

	BSEventNotifyControl ReloadManager::ProcessEvent(const TESCellFullyLoadedEvent* evn, BSTEventSource<TESCellFullyLoadedEvent>* dispatcher)
	{
		GtsManager::GetSingleton().reapply();
		return BSEventNotifyControl::kContinue;
	}

	BSEventNotifyControl ReloadManager::ProcessEvent(const TESCellAttachDetachEvent* evn, BSTEventSource<TESCellAttachDetachEvent>* dispatcher)
	{
		GtsManager::GetSingleton().reapply();
		return BSEventNotifyControl::kContinue;
	}

	BSEventNotifyControl ReloadManager::ProcessEvent(const TESEquipEvent* evn, BSTEventSource<TESEquipEvent>* dispatcher)
	{
		if (evn) {
			auto* actor = TESForm::LookupByID<Actor>(evn->actor->formID);
			if (actor) {
				GtsManager::GetSingleton().reapply_actor(actor);
			}
		}
		return BSEventNotifyControl::kContinue;
	}

	BSEventNotifyControl ReloadManager::ProcessEvent(const MenuOpenCloseEvent* a_event, BSTEventSource<MenuOpenCloseEvent>* a_eventSource)
	{
		auto mName = a_event->menuName;

		if (
			mName == RE::JournalMenu::MENU_NAME ||
			mName == RE::InventoryMenu::MENU_NAME ||
			mName == RE::MapMenu::MENU_NAME ||
			mName == RE::BookMenu::MENU_NAME ||
			mName == RE::LockpickingMenu::MENU_NAME ||
			mName == RE::MagicMenu::MENU_NAME ||
			mName == RE::RaceSexMenu::MENU_NAME ||
			mName == RE::CraftingMenu::MENU_NAME ||
			mName == RE::SleepWaitMenu::MENU_NAME ||
			mName == RE::TrainingMenu::MENU_NAME ||
			mName == RE::BarterMenu::MENU_NAME ||
			mName == RE::FavoritesMenu::MENU_NAME ||
			mName == RE::GiftMenu::MENU_NAME ||
			mName == RE::StatsMenu::MENU_NAME ||
			mName == RE::ContainerMenu::MENU_NAME ||
			mName == RE::DialogueMenu::MENU_NAME ||
			mName == RE::MessageBoxMenu::MENU_NAME ||
			mName == RE::TweenMenu::MENU_NAME || // tab menu
			mName == RE::MainMenu::MENU_NAME ||
			mName == "CustomMenu") { // papyrus custom menues go here
			if (a_event->opening) {
				DebugOverlayMenu::Hide(mName.c_str());
			} else {
				DebugOverlayMenu::Show(mName.c_str());
			}
		}
		// for some reason, after each cell change, the menu is hidden and needs to be shown again
		// using a UI message kShow
		else if (mName == RE::LoadingMenu::MENU_NAME) {
			if (!a_event->opening) {
				DebugOverlayMenu::Load();
			}
		}

		return RE::BSEventNotifyControl::kContinue;
	}
}
