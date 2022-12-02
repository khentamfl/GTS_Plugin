#include "managers/RandomGrowth.hpp"
#include "managers/GrowthTremorManager.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/InputManager.hpp"
#include "managers/vore.hpp"
#include "managers/altcamera.hpp"
#include "magic/effects/common.hpp"
#include "util.hpp"
#include "scale/scale.hpp"
#include "data/persistent.hpp"
#include "data/runtime.hpp"
#include "data/time.hpp"
#include "timer.hpp"
#include "util.hpp"

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
		bool ShiftPressed = false;
		bool LeftArrow = false;
		bool RightArrow = false;
		bool E_Pressed = false;
		bool V_Pressed = false;
		bool F_Pressed = false;

		bool ArrowUp = false;
		bool ArrowDown = false;
		auto player = PlayerCharacter::GetSingleton();
		auto caster = player;
		auto VoreManager = Vore::GetSingleton();
		float size = get_target_scale(player);


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
				auto Cache = Runtime::GetFloat("ManualGrowthStorage");


				//log::info("Time Elapsed: {}, Cache Value: {}", Time::WorldTimeElapsed(), Cache);
				if (key == 0x12 && Cache > 0.0) { // E
					this->TickCheck += 1.0;
					GrowthTremorManager::GetSingleton().CallRumble(caster, caster, Cache/15 * buttonEvent->HeldDuration());
					if (this->timergrowth.ShouldRunFrame()) {
						Runtime::PlaySound("growthSound", caster, Cache/25 * buttonEvent->HeldDuration(), 0.0);
					}

					if (this->timer.ShouldRun() && buttonEvent->HeldDuration() >= 1.2 && Runtime::HasPerk(caster, "SizeReserve")) {
						float gigantism = 1.0 + SizeManager::GetSingleton().GetEnchantmentBonus(caster)/100;
						this->TickCheck = 0.0;
						float Volume = clamp(0.10, 2.0, get_visual_scale(caster) * Cache);
						Runtime::PlaySound("growthSound", caster, Volume, 0.0);
						Runtime::PlaySound("MoanSound", caster, Volume, 0.0);
						RandomGrowth::GetSingleton().CallShake(Cache);
						mod_target_scale(caster, Cache * gigantism);
						Cache = 0.0;
					}
				}
				if (key == 0x21 && buttonEvent->HeldDuration() >= 1.2 && this->timer.ShouldRun() && Runtime::HasPerk(caster, "SizeReserve")) { //F

					float gigantism = 1.0 + SizeManager::GetSingleton().GetEnchantmentBonus(caster)/100;
					float Value = Cache * gigantism;
					Notify("Reserved Size: {}", Value);
					//ConsoleLog::GetSingleton()->Print("tfc");
				}

				if (key == 0x1d && buttonEvent->HeldDuration() >= 1.2 && this->timer.ShouldRun()) { // Left CTRL
					for (auto actor: find_actors()) {
						if (actor->formID != 0x14 && Runtime::InFaction(actor, "FollowerFaction") || actor->IsPlayerTeammate()) {
							float gigantism = SizeManager::GetSingleton().GetEnchantmentBonus(actor)/100;
							float Scale = get_target_scale(actor);
							float MaxScale = get_max_scale(actor);
							// ConsoleLog::GetSingleton()->Print("%s Scale is: %g; Size Limit is: %g; Aspect Of Giantess: %g", actor->GetDisplayFullName(), Scale, MaxScale, gigantism);
						}
					}
				}

				if (key == 0x38) {
					AltPressed = true;
					// log::info("ALT");
				} else if (key == 0x1D) {
					CtrlPressed = true;
				} else if (key == 0xCD) {
					// log::info("RIGHT");
					RightArrow = true;
				} else if (key == 0xCB) {
					// log::info("LEFT");
					LeftArrow = true;
				} else if (key == 0xC8) {
					// log::info("UP");
					ArrowUp = true;
				} else if (key == 0xD0) {
					// log::info("DOWN");
					ArrowDown = true;
				} else if (key == 0x2A) {
					ShiftPressed = true;
				} else if (key == 0x12) {
					E_Pressed = true;
				} else if (key == 0x2F) {
					V_Pressed = true;
				} else if (key == 0x21) {
					F_Pressed = true;
				}
			} else if (buttonEvent->device.get() == INPUT_DEVICE::kMouse && this->timer.ShouldRun()) {
				auto key = buttonEvent->GetIDCode();
				if (key == 0x1 && buttonEvent->HeldDuration() <= 0.025) {
					//player->NotifyAnimationGraph("JumpLand");
					// Attack Right
				}
				if (key == 0x2 && buttonEvent->HeldDuration() <= 0.025) {
					// Do attack left
				}
			}
		}

		Actor* pred = PlayerCharacter::GetSingleton();
		if (Runtime::HasPerk(pred, "VorePerk")) {
			if (ShiftPressed && V_Pressed && !this->voreBlock) {
				if (voretimer.ShouldRunFrame()) {
					this->voreBlock = true;

					std::size_t numberOfPrey = 1;
					if (Runtime::HasPerk(pred, "MassVorePerk")) {
						numberOfPrey = 3;
					}
					std::vector<Actor*> preys = VoreManager.GetVoreTargetsInFront(pred, numberOfPrey);
					for (auto prey: preys) {
						VoreManager.StartVore(pred, prey);
					}
				}
			} else if (!ShiftPressed && !V_Pressed) {
				this->voreBlock = false;
			}
		}

		auto& Camera = CameraManager::GetSingleton();
		if (AltPressed == true && RightArrow == true && LeftArrow == true) {
			Camera.ResetLeftRight();
		}
		if (AltPressed == true && RightArrow == true) {
			Camera.AdjustLeftRight(0.6 + (size * 0.0125 - 0.0125));
		}
		if (AltPressed == true && LeftArrow == true) {
			Camera.AdjustLeftRight(-0.6 + (size * 0.0125 - 0.0125));
		} // Left or Right end


		if (AltPressed == true && ArrowDown == true && ArrowUp == true) {
			Camera.ResetUpDown();
		}
		if (AltPressed == true && ArrowUp == true) {
			Camera.AdjustUpDown(0.6 + (size * 0.0125 - 0.0125));
		}
		if (AltPressed == true && ArrowDown == true) {
			Camera.AdjustUpDown(-0.6 + (size * 0.0125 - 0.0125));
			//log::info("Alt + Down");
		} // Up or Down end

		if (Runtime::HasPerk(player, "TotalControl") && !ShiftPressed && !AltPressed && ArrowUp && LeftArrow && !ArrowDown) { // Grow self
			float scale = get_visual_scale(player);
			auto caster = player;
			float stamina = clamp(0.05, 1.0, GetStaminaPercentage(caster));
			DamageAV(caster, ActorValue::kStamina, 0.15 * (scale * 0.5 + 0.5) * stamina * TimeScale());
			mod_target_scale(caster, 0.0012 * scale * stamina);
			float Volume = clamp(0.10, 2.0, get_visual_scale(caster)/10);
			GrowthTremorManager::GetSingleton().CallRumble(caster, caster, scale/10);
			if (this->timergrowth.ShouldRun()) {
				Runtime::PlaySound("growthSound", caster, Volume, 0.0);
			}
		}
		if (Runtime::HasPerk(player, "TotalControl") && !ShiftPressed && !AltPressed && ArrowDown && LeftArrow && !ArrowUp) { // Shrink Self
			float scale = get_visual_scale(player);
			auto caster = player;
			float stamina = clamp(0.05, 1.0, GetStaminaPercentage(caster));
			DamageAV(caster, ActorValue::kStamina, 0.10 * (scale * 0.5 + 0.5) * stamina * TimeScale());
			mod_target_scale(caster, -0.0012 * scale * stamina);
			float Volume = clamp(0.05, 2.0, get_visual_scale(caster)/10);
			GrowthTremorManager::GetSingleton().CallRumble(caster, caster, scale/14);
			if (this->timergrowth.ShouldRun()) {
				Runtime::PlaySound("shrinkSound", caster, Volume, 0.0);
			}
		} else if (Runtime::HasPerk(player, "TotalControl") && ShiftPressed && !AltPressed && ArrowUp && LeftArrow && !ArrowDown) { // Grow Ally
			for (auto actor: find_actors()) {
				if (!actor) {
					continue;
				}

				if (actor->formID != 0x14 && (actor->IsPlayerTeammate() || Runtime::InFaction(actor, "FollowerFaction"))) {
					float scale = get_visual_scale(actor);
					auto caster = player;
					auto target = actor;
					float magicka = clamp(0.05, 1.0, GetMagikaPercentage(target));
					DamageAV(caster, ActorValue::kMagicka, 0.15 * (scale * 0.5 + 0.5) * magicka * TimeScale());
					mod_target_scale(target, 0.0012 * scale * magicka);
					float Volume = clamp(0.05, 2.0, get_visual_scale(target)/10);
					GrowthTremorManager::GetSingleton().CallRumble(target, caster, 0.25);
					if (this->timergrowth.ShouldRun()) {
						Runtime::PlaySound("growthSound", target, Volume, 0.0);
					}
				}
			}
		} else if (Runtime::HasPerk(player, "TotalControl") && ShiftPressed && !AltPressed && ArrowDown && LeftArrow && !ArrowUp) { // Shrink Ally
			for (auto actor: find_actors()) {
				if (!actor) {
					continue;
				}

				if (actor->formID != 0x14 && (actor->IsPlayerTeammate() || Runtime::InFaction(actor, "FollowerFaction"))) {
					float scale = get_visual_scale(actor);
					auto caster = player;
					auto target = actor;
					float magicka = clamp(0.05, 1.0, GetMagikaPercentage(target));
					DamageAV(target, ActorValue::kMagicka, 0.10 * (scale * 0.5 + 0.5) * magicka * TimeScale());
					mod_target_scale(target, -0.0012 * scale * magicka);
					float Volume = clamp(0.05, 2.0, get_visual_scale(target)/10);
					GrowthTremorManager::GetSingleton().CallRumble(target, caster, 0.20);
					if (this->timergrowth.ShouldRun()) {
						Runtime::PlaySound("shrinkSound", target, Volume, 0.0);
					}
				}
			}
		}


		return BSEventNotifyControl::kContinue;
	}

	std::string InputManager::DebugName() {
		return "InputManager";
	}

	void InputManager::Start() {
		auto deviceManager = RE::BSInputDeviceManager::GetSingleton();
		deviceManager->AddEventSink(&InputManager::GetSingleton());
	}
}
