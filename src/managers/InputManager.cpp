

#include "managers/RandomGrowth.hpp"
#include "managers/GrowthTremorManager.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/InputManager.hpp"
#include "magic/effects/common.hpp"
#include "util.hpp"
#include "scale/scale.hpp"
#include "data/persistent.hpp"
#include "data/runtime.hpp"
#include "data/time.hpp"
#include "timer.hpp"

using namespace RE;
using namespace Gts;


namespace Gts {
	InputManager& InputManager::GetSingleton() noexcept {
		static InputManager instance;
		return instance;
	}

	EventResult InputManager::ProcessEvent(RE::InputEvent* const* a_event, RE::BSTEventSource<RE::InputEvent*> a_source) {
		if (!a_event) {
			return EventResult::kContinue;
		}
		for (auto event = *a_event; event; event = event->next) {
			if (event->GetEventType() != INPUT_EVENT_TYPE::kButton) {
				continue;
			}
			ButtonEvent* buttonEvent = a_event->AsButtonEvent();
			if (!buttonEvent) {
				continue;
			}

			if ((buttonEvent->idCode == 0x45) && buttonEvent->IsPressed()) {
				// Do attack
				log::info("0x45 pressed");
			} else if ((buttonEvent->idCode == 0x1) && buttonEvent->IsPressed()) {
				// Do attack left
				log::info("0x01 pressed");
			} else if ((buttonEvent->idCode == 0x2) && buttonEvent->IsPressed()) {
				// Do attack right
				log::info("0x02 pressed");
			}
		}
		return EventResult::kContinue;
	}

	void InputManager::Start() {
		auto deviceManager = RE::BSInputDeviceManager::GetSingleton();
		deviceManager->AddEventSink(&InputManager::GetSingleton());
	}
}
