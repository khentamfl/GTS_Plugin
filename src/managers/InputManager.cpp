

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

	BSEventNotifyControl InputManager::ProcessEvent(InputEvent* const* a_event, BSTEventSource<InputEvent*>* a_eventSource) {
		if (!a_event) {
			return BSEventNotifyControl::kContinue;
		}
		for (auto event = *a_event; event; event = event->next) {
			if (event->GetEventType() != INPUT_EVENT_TYPE::kButton) {
				log::info("ButtonEvent Type != Button");
				continue;
			}
			ButtonEvent* buttonEvent = event->AsButtonEvent();
			if (!buttonEvent || (!buttonEvent->IsPressed() && !buttonEvent->IsUp())) {
				log::info("ButtonEvent aren't pressed");
				continue;

			}
			if (buttonEvent->device.get() == INPUT_DEVICE::kKeyboard) {
				log::info("ButtonEvent == Keyboard");
				auto key = buttonEvent->GetIDCode();
				if (key == GFxKey::kE) {
					// Do attack
					ConsoleLog::GetSingleton()->Print("Pressed E");
					mod_target_scale(PlayerCharacter::GetSingleton(), 0.33);
					log::info("0x45 pressed");
				} else {
					log::info("Pressed: {}", key);
				}
			} else if (buttonEvent->device.get() == INPUT_DEVICE::kMouse) {
				log::info("ButtonEvent == Mouse");
				auto key = buttonEvent->GetIDCode();
				if (key == 0x1) {
					// Do attack left
					ConsoleLog::GetSingleton()->Print("Pressed LMB");
					log::info("0x01 pressed");
				} else if (key == 0x2) {
					// Do attack right
					ConsoleLog::GetSingleton()->Print("Pressed RMB");
					log::info("0x02 pressed");
				} else {
					log::info("Pressed: {}", key);
				}
			}
		}
		return BSEventNotifyControl::kContinue;
	}

	void InputManager::Start() {
		auto deviceManager = RE::BSInputDeviceManager::GetSingleton();
		deviceManager->AddEventSink(&InputManager::GetSingleton());
	}
}
