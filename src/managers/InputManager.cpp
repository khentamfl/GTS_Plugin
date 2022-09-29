

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
#include "data/time.hpp"

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
				auto Cache = runtime.ManualGrowthStorage;
				if (key == 0x12 && Time::WorldTimeElapsed() >= 2.0 && Cache->value > 0.0) {
					// Grow
					if (this->timer.ShouldRun()) {
						auto GrowthSound = runtime.growthSound;
						auto MoanSound = runtime.MoanSound;
						Cache->value = 0.0;
						float Volume = clamp(0.10, 2.0, get_visual_scale(caster) * Cache->value);
						PlaySound(GrowthSound, caster, Volume, 0.0);
						PlaySound(MoanSound, caster, Volume, 0.0);
						GrowthTremorManager::GetSingleton().CallRumble(caster, caster, Cache->value);
						Grow(caster, Cache->value, 0.0);
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
