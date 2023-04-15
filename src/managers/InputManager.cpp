#include "managers/InputManager.hpp"
#include "managers/damage/AccurateDamage.hpp"

#include "managers/animation/AnimationManager.hpp"
#include "managers/CrushManager.hpp"
#include "managers/highheel.hpp"
#include "magic/effects/common.hpp"
#include "scale/scale.hpp"
#include "data/persistent.hpp"
#include "data/transient.hpp"
#include "data/runtime.hpp"
#include "data/plugin.hpp"
#include "data/time.hpp"
#include "timer.hpp"
#include "managers/Rumble.hpp"

using namespace RE;
using namespace Gts;

namespace {
	std::vector<InputEventData> LoadInputEvents() {
		const auto data = toml::parse(R"(Data\SKSE\Plugins\GtsInput.toml)");
		// Toml Example
		// ```toml
		// [[InputEvent]]
		// name = “Stomp”
		// keys = [“E”, “LeftShift”]
		// duration = 0.0
		// ```
		const auto aov = toml::find_or<std::vector<toml::value> >(data, "InputEvent", {});
		std::vector<InputEventData> results;
		for (const auto& table: aov) {
			std::string name = toml::find_or<std::string>(table, "name", "");
			const auto keys = toml::find_or<vector<std::string> >(table, "keys", {});
			if (name != "" && !keys.empty()) {
				InputEventData newData(table);
				if (newData.HasKeys()) {
					results.push_back(newData);
				} else {
					log::error("No valid keys found for event {} at line {}", name, table.location().line());
				}
			} else if (keys.empty()) {
				log::warn("Missing keys for {} at line {}", name, table.location().line());
			} else {
				log::warn("Missing name for [[InputEvent]] at line {}", table.location().line());
			}
		}
		return results;
	}

	void SizeReserveEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		auto Cache = Persistent::GetSingleton().GetData(player);
		if (!Cache) {
			return;
		}
		if (Cache->SizeReserve > 0.0) {
			float duration = data.Duration();
			Rumble::Once("SizeReserve", player, Cache->SizeReserve/15 * duration, 0.05);

			if (duration >= 1.2 && Runtime::HasPerk(player, "SizeReserve") && Cache->SizeReserve > 0) {
				float SizeCalculation = duration - 1.2;
				float gigantism = 1.0 + SizeManager::GetSingleton().GetEnchantmentBonus(player)/100;
				float Volume = clamp(0.10, 2.0, get_visual_scale(player) * Cache->SizeReserve/10);
				static Timer timergrowth = Timer(2.00);
				if (timergrowth.ShouldRunFrame()) {
					Runtime::PlaySound("growthSound", player, Cache->SizeReserve/50 * duration, 0.0);
					Runtime::PlaySoundAtNode("MoanSound", player, Volume, 0.0, "NPC Head [Head]");
				}

				mod_target_scale(player, SizeCalculation/80 * gigantism);
				Cache->SizeReserve -= SizeCalculation/80;
				if (Cache->SizeReserve <= 0) {
					Cache->SizeReserve = 0.0; // Protect against negative values.
				}
			}
		}
	}

	void DisplaySizeReserveEvent(const InputEventData& data) {
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

	void PartyReportEvent(const InputEventData& data) {
		for (auto actor: find_actors()) {
			if (actor->formID != 0x14 && Runtime::InFaction(actor, "FollowerFaction") || actor->IsPlayerTeammate()) {
				float hh = HighHeelManager::GetBaseHHOffset(actor)[2]/100;
				float gigantism = SizeManager::GetSingleton().GetEnchantmentBonus(actor)/100;
				float scale = get_target_scale(actor);
				float maxscale = get_max_scale(actor);
				Cprint("{} Scale is: {:.2f}; Size Limit is: {:.2f}; High Heels: {:.2f}; Aspect Of Giantess: {:.2f}", actor->GetDisplayFullName(), scale, maxscale, hh, gigantism);
			}
		}
	}

	void AnimSpeedUpEvent(const InputEventData& data) {
		AnimationManager::AdjustAnimSpeed(0.025); // Increase speed and power
	}
	void AnimSpeedDownEvent(const InputEventData& data) {
		AnimationManager::AdjustAnimSpeed(-0.060); // Decrease speed and power
	}
	void AnimMaxSpeedEvent(const InputEventData& data) {
		AnimationManager::AdjustAnimSpeed(0.060); // Strongest attack
	}
}

namespace Gts {
	InputEventData::InputEventData(const toml::value& data) {
		this->name = toml::find_or<std::string>(data, "name", "");
		float duration = toml::find_or<float>(data, "duration", 0.0f);
		this->exclusive = toml::find_or<bool>(data, "exclusive", false);
		std::string trigger = toml::find_or<std::string>(data, "trigger", "once");
		std::string lower_trigger = str_tolower(trigger);
		if (lower_trigger == "once") {
			this->trigger = TriggerMode::Once;
		} else if (
			lower_trigger ==  "continuous"
			|| lower_trigger ==  "cont"
			|| lower_trigger ==  "continue") {
			this->trigger = TriggerMode::Continuous;
		} else {
			log::warn("Unknown trigger value: {}", lower_trigger);
			this->trigger = TriggerMode::Once;
		}
		this->minDuration = duration;
		this->startTime = 0.0;
		this->keys = {};
		const auto keys = toml::find_or<vector<std::string> >(data, "keys", {});
		for (const auto& key: keys) {
			std::string upper_key = str_toupper(remove_whitespace(key));
			if (upper_key != "LEFT" && upper_key != "DIK_LEFT") {
				// This changes LEFTALT to LALT
				// But NOT LEFT into L
				replace_first(upper_key, "LEFT", "L");
			}
			if (upper_key != "RIGHT" && upper_key != "DIK_RIGHT") {
				replace_first(upper_key, "RIGHT", "R");
			}
			try {
				std::uint32_t key_code = NAMED_KEYS.at(upper_key);
				this->keys.emplace(key_code);
			} catch (std::out_of_range e) {
				log::warn("Key named {}=>{} in {} was unrecongized.", key, upper_key, this->name);
				this->keys.clear();
				return; // Remove all keys and return so that this becomes an INVALID key entry and won't fire
			}
		}
	}

	float InputEventData::Duration() const {
		return Time::WorldTimeElapsed() - this->startTime;
	}

	bool InputEventData::AllKeysPressed(const std::unordered_set<std::uint32_t>& keys) {
		if (this->keys.empty()) {
			return false;
		}
		for (const auto& key: this->keys) {
			if (keys.find(key) == keys.end()) {
				// Key not found
				return false;
			}
		}
		return true;
	}

	bool InputEventData::OnlyKeysPressed(const std::unordered_set<std::uint32_t>& keys_in) {
		std::unordered_set<std::uint32_t> keys(keys_in); // Copy
		for (const auto& key: this->keys) {
			keys.erase(key);
		}
		return keys.size() == 0;
	}

	bool InputEventData::ShouldFire(const std::unordered_set<std::uint32_t>& keys) {
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

	std::string InputEventData::GetName() {
		return this->name;
	}

	bool InputEventData::HasKeys() {
		return !this->keys.empty();
	}

	RegisteredInputEvent::RegisteredInputEvent(std::function<void(const InputEventData&)> callback) : callback(callback) {

	}

	InputManager& InputManager::GetSingleton() noexcept {
		static InputManager instance;
		return instance;
	}

	void InputManager::RegisterInputEvent(std::string_view namesv, std::function<void(const InputEventData&)> callback) {
		auto& me = InputManager::GetSingleton();
		std::string name(namesv);
		me.registedInputEvents.try_emplace(name, callback);
		log::debug("Registered input event: {}", namesv);
	}

	void InputManager::DataReady() {
		try {
			InputManager::GetSingleton().keyTriggers = LoadInputEvents();
		} catch (toml::exception e) {
			log::error("Error in parsing GtsInput.toml: {}", e.what());
		} catch (std::runtime_error e) {
			log::error("Error in opening GtsInput.toml: {}", e.what());
		} catch (std::exception e) {
			log::error("Error in GtsInput.toml: {}", e.what());
		}
		log::info("Loaded {} key bindings", InputManager::GetSingleton().keyTriggers.size());
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

		// log::trace("Currently {} keys are pressed", keys.size());
		for (auto& trigger: this->keyTriggers) {
			// log::trace("Checking the {} event", trigger.GetName());
			if (trigger.ShouldFire(keys)) {
				log::debug(" - Running event {}", trigger.GetName());
				try {
					auto& eventData = this->registedInputEvents.at(trigger.GetName());
					eventData.callback(trigger);
				} catch (std::out_of_range e) {
					log::warn("Event {} was triggered but there is no event of that name", trigger.GetName());
					continue;
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
