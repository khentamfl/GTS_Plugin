

#include "managers/RandomGrowth.hpp"
#include "managers/GrowthTremorManager.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/InputManager.hpp"
#include "managers/camera.hpp"
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
		bool CtrlPressed = false;
		bool AltPressed = false;
		bool LeftArrow = false;
		bool RightArrow = false;

		bool ArrowUp = false;
		bool ArrowDown = false;


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
				if (!Cache) {
					log::info("Cache is invalid");
					continue;
				}


				//log::info("Time Elapsed: {}, Cache Value: {}", Time::WorldTimeElapsed(), Cache->value);
				if (key == 0x12 && Cache->value > 0.0) {
					this->TickCheck += 1.0;
					GrowthTremorManager::GetSingleton().CallRumble(caster, caster, Cache->value/15 * buttonEvent->HeldDuration());
					PlaySound(runtime.growthSound, caster, Cache->value/25 * buttonEvent->HeldDuration(), 0.0);
					if (this->timer.ShouldRun() && buttonEvent->HeldDuration() >= 1.2) {
						float gigantism = 1.0 + SizeManager::GetSingleton().GetEnchantmentBonus(caster)/100;
						auto GrowthSound = runtime.growthSound;
						auto MoanSound = runtime.MoanSound;
						this->TickCheck = 0.0;
						float Volume = clamp(0.10, 2.0, get_visual_scale(caster) * Cache->value);
						PlaySound(GrowthSound, caster, Volume, 0.0);
						PlaySound(MoanSound, caster, Volume, 0.0);
						RandomGrowth::GetSingleton().CallShake(Cache->value);
						mod_target_scale(caster, Cache->value * gigantism);
						Cache->value = 0.0;
					}
				}
				if (key == 0x21 && buttonEvent->HeldDuration() >= 1.2 && this->timer.ShouldRun() && caster->HasPerk(runtime.SizeReserve)) {
					float gigantism = 1.0 + SizeManager::GetSingleton().GetEnchantmentBonus(caster)/100;
					float Value = Cache->value * gigantism;
					Notify("Reserved Size: {}", Value);
				}
				if (key == 0x2E) {
					auto Player = PlayerCharacter::GetSingleton();
					actor_data = Persistent::GetSingleton().GetData(Player);
				if (actor_data) {
					actor_data->anim_speed -=0.001;
					}
				}

				if (key == 0x38) {
					AltPressed = true;
				} else if (key == 0x1D) {
					CtrlPressed = true;
				} else if (key == 0xCD) {
					RightArrow = true;
				} else if (key == 0xCB) {
					LeftArrow = true;
				} else if (key == 0xC8) {
					ArrowUp = true;
				} else if (key == 0xD0) {
					ArrowDown = true;
				}
			} else if (buttonEvent->device.get() == INPUT_DEVICE::kMouse) {
				auto key = buttonEvent->GetIDCode();
				if (key == 0x1 && buttonEvent->HeldDuration() <= 0.05) {
					PlayerCharacter::GetSingleton()->NotifyAnimationGraph("JumpLand");
					// Do attack right
					// ConsoleLog::GetSingleton()->Print("Pressed LMB");
				}
				if (key == 0x2 && buttonEvent->HeldDuration() <= 0.05) {
					// Do attack left
					// ConsoleLog::GetSingleton()->Print("Pressed RMB");
				}
				// log::info("{:X} pressed", key);
			}
		}

		auto Camera = CameraManager::GetSingleton();
		if (AltPressed == true && RightArrow == true && LeftArrow == true) {
			Camera.AdjustSide(true, false, false); // Reset
			log::info("Alt + Left & Right: Reset");
		}
		if (AltPressed == true && RightArrow == true) {
			Camera.AdjustSide(false, true, false); // Right
			log::info("Alt + Right");
		}
		if (AltPressed == true && LeftArrow == true) {
			Camera.AdjustSide(false, false, true); // Left
			log::info("Alt + Right");
		} // Left or Right end


		if (AltPressed == true && ArrowDown == true && ArrowUp == true) {
			Camera.AdjustUpDown(true, false, false); // Reset
			log::info("Alt + Up & Down: Reset");
		}
		if (AltPressed == true && ArrowUp == true) {
			Camera.AdjustUpDown(false, true, false); // Up
			log::info("Alt + Up");
		}
		if (AltPressed == true && ArrowDown == true) {
			Camera.AdjustUpDown(false, false, true); // Down
			log::info("Alt + Down");
		} // Up or Down end

		return BSEventNotifyControl::kContinue;
	}

	void InputManager::Start() {
		auto deviceManager = RE::BSInputDeviceManager::GetSingleton();
		deviceManager->AddEventSink(&InputManager::GetSingleton());
	}
}
