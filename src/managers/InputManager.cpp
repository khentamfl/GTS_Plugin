#include "managers/damage/AccurateDamage.hpp"
#include "managers/GrowthTremorManager.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/animation/Grab.hpp"
#include "managers/animation/ThighCrush.hpp"
#include "managers/CrushManager.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/RandomGrowth.hpp"
#include "managers/InputManager.hpp"
#include "managers/highheel.hpp"
#include "managers/vore.hpp"
#include "managers/camera.hpp"
#include "magic/effects/common.hpp"
#include "scale/scale.hpp"
#include "data/persistent.hpp"
#include "data/transient.hpp"
#include "data/runtime.hpp"
#include "data/plugin.hpp"
#include "data/time.hpp"
#include "timer.hpp"
#include "managers/Rumble.hpp"
#include "toml.hpp"

using namespace articuno;
using namespace RE;
using namespace Gts;

namespace {
  // courtesy of https://stackoverflow.com/questions/5878775/how-to-find-and-replace-string
  void replace_first(
    std::string& s,
    std::string const& toReplace,
    std::string const& replaceWith
  ) {
      std::size_t pos = s.find(toReplace);
      if (pos == std::string::npos) return;
      s.replace(pos, toReplace.length(), replaceWith);
  }

  std::vector<InputEvent> LoadInputEvents() {
    const auto data = toml::parse(R"(Data\SKSE\Plugins\GtsInput.toml)");
    // Toml Example
    // ```toml
    // [[InputEvent]]
    // name = “Stomp”
    // keys = [“E”, “LeftShift”]
    // duration = 0.0
    // ```
    const auto aot = toml::find<std::vector<toml::table>>(data, "InputEvent");
    std::vector<InputEvent> results;
    for (const auto& table: aot) {
      std::string name = toml::find_or<std::string>(data, "name", "");
      const auto keys = toml::find_or<vector<std::string>>(data, "keys", {});
      if (name != "" && ! keys.empty()) {
        InputEvent newData = InputEvent(data);
        results.push_back(newData);
      }
    }
    return results;
  }

  void SizeReserveEvent(const InputEvent& data) {
    auto player = PlayerCharacter::GetSingleton();
    auto Cache = Persistent::GetSingleton().GetData(player);
    if (!Cache) {
      return BSEventNotifyControl::kContinue;
    }
    if (Cache->SizeReserve > 0.0) {
      float duration = data.Duration();
      Rumble::Once("SizeReserve", player, Cache->SizeReserve/15 * duration);

      if (duration >= 1.2 && Runtime::HasPerk(player, "SizeReserve") && Cache->SizeReserve > 0) {
        float SizeCalculation = duration - 1.2;
        float gigantism = 1.0 + SizeManager::GetSingleton().GetEnchantmentBonus(player)/100;
        float Volume = clamp(0.10, 2.0, get_visual_scale(player) * Cache->SizeReserve/10);
        static timergrowth = Timer(2.00);
        if (timergrowth.ShouldRunFrame()) {
          Runtime::PlaySound("growthSound", player, Cache->SizeReserve/50 * duration, 0.0);
          Runtime::PlaySound("MoanSound", player, Volume, 0.0);
        }

        mod_target_scale(player, SizeCalculation/80 * gigantism);
        Cache->SizeReserve -= SizeCalculation/80;
        if (Cache->SizeReserve <= 0) {
          Cache->SizeReserve = 0.0; // Protect against negative values.
        }
      }
    }
  }

  void DisplaySizeReserveEvent(const InputEvent& data) {
    auto player = PlayerCharacter::GetSingleton();
    auto Cache = Persistent::GetSingleton().GetData(player);
    if (Cache) {
      if (Runtime::HasPerk(player, "SizeReserve")) { //F

        float gigantism = 1.0 + SizeManager::GetSingleton().GetEnchantmentBonus(player)/100;
        float Value = Cache->SizeReserve * gigantism;
        Notify("Reserved Size: {:.2f}", Value);
      }
    }
  }

  void PartyReportEvent(const InputEvent& data) {
    for (auto actor: find_actors()) {
      if (actor->formID != 0x14 && Runtime::InFaction(actor, "FollowerFaction") || actor->IsPlayerTeammate()) {
        float hh = HighHeelManager::GetBaseHHOffset(actor)[2]/100;
        float gigantism = SizeManager::GetSingleton().GetEnchantmentBonus(actor)/100;
        float scale = get_target_scale(actor);
        float maxscale = get_max_scale(actor);
        ConsoleLog::GetSingleton()->Print("%s Scale is: %g; Size Limit is: %g; High Heels: %g; Aspect Of Giantess %: %g", actor->GetDisplayFullName(), scale, maxscale, hh, gigantism);
      }
    }
  }

  void AnimSpeedUpEvent(const InputEvent& data) {
    AnimationManager::AdjustAnimSpeed(0.012); // Increase speed and power
  }
  void AnimSpeedDownEvent(const InputEvent& data) {
    AnimationManager::AdjustAnimSpeed(-0.0060); // Decrease speed and power
  }
  void AnimMaxSpeedEvent(const InputEvent& data) {
    AnimationManager::AdjustAnimSpeed(0.030); // Strongest attack
  }
}

namespace Gts {
  InputEvent::InputEvent(const toml::table& data) {
    this->name = toml::find_or<float>(data, "name", "");
    float duration = toml::find_or<float>(data, "duration", 0.0f);
    this->exclusive = toml::find_or<bool>(data, "exclusive", false);
    std::string trigger = toml::find_or<bool>(data, "trigger", "once");
    std::string lower_trigger = std::transform(trigger.begin(), trigger.end(), trigger.begin(), [](unsigned char c){ return std::tolower(c); }); // Lowercase
    switch (lower_trigger) {
      case "once": {
        this->trigger = TriggerMode::Once;
        break;
      },
      case "continuous": {
        this->trigger = TriggerMode::Continuous;
        break;
      }
      case "cont": {
        this->trigger = TriggerMode::Continuous;
        break;
      }
      case "continue": {
        this->trigger = TriggerMode::Continuous;
        break;
      }
      default: {
        this->trigger = TriggerMode::Once;
        break;
      }
    }
    this->keys = {};
    const auto keys = toml::find_or<vector<std::string>>(data, "keys", {});
    for (const auto& key: keys) {
      std::string upper_key = std::transform(key.begin(), key.end(), key.begin(), [](unsigned char c){ return std::toupper(c); }); // Uppercase
      upper_key.erase(remove(upper_key.begin(),upper_key.end(),' '),str.end()); // Remove spaces
      if (upper_key != "LEFT" && upper_key != "DIK_LEFT") {
        // This changes LEFTALT to LALT
        // But NOT LEFT into L
        replace_first(upper_key, "LEFT", "L");
      }
      if (upper_key != "RIGHT" && upper_key != "DIK_RIGHT") {
        replace_first(upper_key, "RIGHT", "R");
      }
      this->keys.push_back(upper_key);
    }
    if (this->keys.empty()) {
      continue;
    }
    this->minDuration = duration;
    this->startTime = 0.0;
  }

  float InputEvent::Duration() {
    return Time::WorldTimeElapsed() - this->startTime;
  }

  bool InputEvent::AllKeysPressed(const std::unordered_set<std::uint32_t>& keys) {
    for (const auto& key: this->keys) {
      if (keys.find(upper_key) == keys.end()) {
        // Key not found
        return false;
      }
    }
    return true;
  }

  bool InputEvent::OnlyKeysPressed(const std::unordered_set<std::uint32_t>& keys_in) {
    std::unordered_set<std::uint32_t> keys = keys_in; // Copy
    for (const auto& key: this->keys) {
      keys.erase(key);
    }
    return keys.size() == 0;
  }

  bool InputEvent::ShouldFire(const std::unordered_set<std::uint32_t>& keys) {
    bool shouldFire = false;
    // Check based on keys and duration
    if (this->AllKeysPressed(keys) && (!this->exclusive || this->OnlyKeysPressed(keys))) {
      shouldFire = true;
    } else {
      // Keys aren't held reset the start time of the button hold
      this->startTime = Time::WorldTimeElapsed();
      // and reset the state to idle
      this->state = InputEventState::Idle;
    }
    // Check based on duration
    if (shouldFire) {
      if (this->minDuration > 0.0) {
        // Turn it off if duration is not met
        shouldFire = this->Duration() > this->minDuration;
      }
    }
    // Check based on held and trigger state
    if (shouldFire) {
      switch (this->state) {
        case InputEventState::Idle: {
          this->state = InputEventState::Held;
          return true;
        }
        case InputEventState::Held: {
          switch (this->trigger) {
            case TriggerMode::Once: {
              return false;
            }
            case TriggerMode::Continuous: {
              return true;
            }
            default: {
              log::error("Unexpected TriggerMode.");
              return false; // Catch if something goes weird
            }
          }
        }
        default: {
          log::error("Unexpected InputEventState.");
          return false; // Catch if something goes weird
        }
      }
    } else {
      return false;
    }
  }

  RegisteredInputEvent::RegisteredInputEvent(std::function<void(InputEvent&)> callback) : callback(callback) {

  }

	InputManager& InputManager::GetSingleton() noexcept {
		static InputManager instance;
		return instance;
	}

  void InputManager::RegisterInputEvent(std::string_view name, std::function<void(const InputEvent&)> callback) {
    auto& me = InputManager::GetSingleton();
    me.registedInputEvents.try_emplace(name, callback);
  }

  void InputManager::DataReady() {
    InputManager::RegisterInputEvent("SizeReserve", SizeReserveEvent);
    InputManager::RegisterInputEvent("DisplaySizeReserve", DisplaySizeReserveEvent);
    InputManager::RegisterInputEvent("PartyReport", PartyReportEvent);
    InputManager::RegisterInputEvent("AnimSpeedUp", AnimSpeedUpEvent);
    InputManager::RegisterInputEvent("AnimSpeedDown", AnimSpeedDownEvent);
    InputManager::RegisterInputEvent("AnimMaxSpeed", AnimMaxSpeedEvent);
  }

	BSEventNotifyControl InputManager::ProcessEvent(InputEvent* const* a_event, BSTEventSource<InputEvent*>* a_eventSource) {

		if (!a_event) {
			return BSEventNotifyControl::kContinue;
		}
		auto player = PlayerCharacter::GetSingleton();
		if (!player) {
			return BSEventNotifyControl::kContinue;
		}
		if (!Plugin::Live()) {
			return BSEventNotifyControl::kContinue;
		}

    std::unordered_set<std::uint32_t> keys;
		for (auto event = *a_event; event; event = event->next) {
			if (event->GetEventType() != INPUT_EVENT_TYPE::kButton) {
				continue;
			}
			ButtonEvent* buttonEvent = event->AsButtonEvent();
			if (!buttonEvent || (!buttonEvent->IsPressed() && !buttonEvent->IsUp())) {
				continue;
			}

			if (buttonEvent->device.get() == INPUT_DEVICE::kKeyboard) {
				auto key = buttonEvent->GetIDCode();
        keys.emplace(key);
      } else if (buttonEvent->device.get() == INPUT_DEVICE::kMouse) {
        auto key = buttonEvent->GetIDCode();
        keys.emplace(key + MOUSE_OFFSET);
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
