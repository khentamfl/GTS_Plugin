

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
				continue;
			}
			ButtonEvent* buttonEvent = event->AsButtonEvent();
			if (!buttonEvent || (!buttonEvent->IsPressed() && !buttonEvent->IsUp())) {
				continue;

			}
			if (buttonEvent->device.get() == INPUT_DEVICE::kKeyboard) {
				// log::info("ButtonEvent == Keyboard");
				auto key = buttonEvent->GetIDCode();
				auto caster = PlayerCharacter::GetSingleton();
				auto runtime = Runtime::GetSingleton();
				if (key == 0x12) {
					// Grow
					if (this->timer.ShouldRun()) {
						auto GrowthSound = runtime.growthSound;
						float Volume = clamp(0.25, 2.0, get_visual_scale(caster)/2);
						PlaySound(GrowthSound, caster, Volume, 0.0);
						GrowthTremorManager::GetSingleton().CallRumble(caster, caster, 1.0);
						Grow(caster, 3.0, 0.0);
						}
					}
				}  
			if (buttonEvent->device.get() == INPUT_DEVICE::kMouse) {
				auto key = buttonEvent->GetIDCode();
				if (key == 0x1) {
					PlayerCharacter::GetSingleton()->NotifyAnimationGraph("JumpLand");
					// Do attack left
					// ConsoleLog::GetSingleton()->Print("Pressed LMB");
				} if (key == 0x2) {
					// Do attack right
					// ConsoleLog::GetSingleton()->Print("Pressed RMB");
				}
				// log::info("{:X} pressed", key);
			}
		}
		return BSEventNotifyControl::kContinue;
	}

	void InputManager::Start() {
		auto deviceManager = RE::BSInputDeviceManager::GetSingleton();
		deviceManager->AddEventSink(&InputManager::GetSingleton());
	}
}
