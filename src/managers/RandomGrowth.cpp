#include "managers/RandomGrowth.hpp"

#include "managers/GtsSizeManager.hpp"
#include "managers/GtsManager.hpp"
#include "magic/effects/common.hpp"
#include "scale/scale.hpp"
#include "data/persistent.hpp"
#include "data/runtime.hpp"
#include "data/time.hpp"
#include "timer.hpp"
#include "managers/Rumble.hpp"

using namespace RE;
using namespace Gts;

namespace {
	bool ShouldGrow() {
		auto Player = PlayerCharacter::GetSingleton();
		float MultiplySlider = Runtime::GetFloat("RandomGrowthMultiplyPC");
		if (!Runtime::HasPerk(Player, "GrowthPerk") || MultiplySlider == 0) {
			return false;
		}
		int RNG = rand()% 49 + 1;
		RandomGrowth::GetSingleton().RandomizeGrowth(RNG);

		if (SizeManager::GetSingleton().BalancedMode() == 2.0) {
			MultiplySlider = 1.0; // Disable effect in Balance Mode, so slider is always 1.0
		}
		float Gigantism = 1.0 + SizeManager::GetSingleton().GetEnchantmentBonus(Player)/100;
		int Requirement = ((250 * MultiplySlider * SizeManager::GetSingleton().BalancedMode()) / Gigantism); // Doubles random in Balance Mode
		int random = rand() % Requirement;
		int decide_chance = 1;
		if (random <= decide_chance) {
			return true;
		} else {
			return false;
		}
	}

	void RestoreStats() { // Regenerate attributes
		auto Player = PlayerCharacter::GetSingleton();
		if (Runtime::HasPerk(Player, "GrowthAugmentation")) {
			float HP = GetMaxAV(Player, ActorValue::kHealth) * 0.00185;
			float MP = GetMaxAV(Player, ActorValue::kMagicka) * 0.00095;
			float SP = GetMaxAV(Player, ActorValue::kStamina) * 0.00125;
			Player->AsActorValueOwner()->RestoreActorValue(ACTOR_VALUE_MODIFIER::kDamage, ActorValue::kHealth, HP * TimeScale());
			Player->AsActorValueOwner()->RestoreActorValue(ACTOR_VALUE_MODIFIER::kDamage, ActorValue::kMagicka, SP * TimeScale());
			Player->AsActorValueOwner()->RestoreActorValue(ACTOR_VALUE_MODIFIER::kDamage, ActorValue::kStamina, MP * TimeScale());
		}
	}
}

namespace Gts {
	RandomGrowth& RandomGrowth::GetSingleton() noexcept {
		static RandomGrowth instance;
		return instance;
	}
	void RandomGrowth::CallShake(float value) {
		this->CallInputGrowth = true;
		this->ShakePower = value;
	}

	std::string RandomGrowth::DebugName() {
		return "RandomGrowth";
	}

	void RandomGrowth::Update() {
		auto player = PlayerCharacter::GetSingleton();

		if (!player) {
			return;
		}
		if (!player->Is3DLoaded()) {
			return;
		}

		bool hasSMT = Runtime::HasMagicEffect(player, "SmallMassiveThreat");

		if (this->CallInputGrowth == true) { // Shake the screen, play sounds

			auto& Persist = Persistent::GetSingleton();
			auto actor_data = Persist.GetData(player);
			float delta_time = Time::WorldTimeDelta();

			this->growth_time_input += delta_time;
			actor_data->half_life = 1.0 + this->ShakePower/6;

			Rumble::Once("RandomGrowth", player, this->ShakePower * 15, 0.05);

			static Timer timer = Timer(0.33);
			if (timer.ShouldRunFrame() && this->ShakePower > 6.0) {
				Runtime::PlaySound("xlRumbleL", player, this->ShakePower/10, 0.0);
			}
			if (this->growth_time_input >= actor_data->half_life) { // Time in seconds" 160tick / 60 ticks per secong ~= 2.6s
				this->CallInputGrowth = false;
				this->growth_time_input = 0.0;
				actor_data->half_life = 1.0;
				// End growing
			}
		}

		if (this->AllowGrowth == false) {
			static Timer timer = Timer(3.0); // Run every 3.0s or as soon as we can
			if (timer.ShouldRun()) {
				if (ShouldGrow()) {
					// Start growing
					this->growth_time = 0.0;
					this->AllowGrowth = true;
					// Play sound
					Rumble::Once("RandomGrowth", player, 6.0, 0.05);
					float Volume = clamp(0.25, 2.0, get_visual_scale(player)/4);
					Runtime::PlaySoundAtNode("MoanSound", player, 1.0, 0.0, "NPC Head [Head]");
					Runtime::PlaySound("growthSound", player, Volume, 0.0);
				}
			}

		} else if (this->AllowGrowth == true && !hasSMT) {
			// Do the growing
			float delta_time = Time::WorldTimeDelta();
			int TotalPower = (100 + this->Randomize)/100;
			float Scale = get_visual_scale(player);
			float ProgressionMultiplier = Persistent::GetSingleton().progression_multiplier;
			float base_power = ((0.00185 * TotalPower * 60.0 * Scale) * ProgressionMultiplier);  // Put in actual power please
			RestoreStats(); // Regens Attributes if PC has perk
			mod_target_scale(player, base_power * delta_time); // Use delta_time so that the growth will be the same regardless of fps
			Rumble::Once("RandomGrowth", player, base_power * 180, 0.05);
			this->growth_time += delta_time;
			if (this->growth_time >= 2.0) { // Time in seconds" 160tick / 60 ticks per secong ~= 2.6s
				// End growing
				this->AllowGrowth = false;
			}
		}
	}
	void RandomGrowth::RandomizeGrowth(float value) {
		this->Randomize = value;
	}
}
