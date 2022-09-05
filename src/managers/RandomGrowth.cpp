#include "managers/RandomGrowth.hpp"
#include "managers/GrowthTremorManager.hpp"
#include "managers/GtsManager.hpp"
#include "magic/effects/common.hpp"
#include "util.hpp"
#include "scale/scale.hpp"
#include "data/persistent.hpp"
#include "data/runtime.hpp"
#include "data/time.hpp"
#include "timer.hpp"
#include "managers/GtsSizeManager.hpp"

using namespace RE;
using namespace Gts;

namespace {
	bool ShouldGrow() {
		auto& runtime = Runtime::GetSingleton();
		auto Player = PlayerCharacter::GetSingleton();
		float Gigantism = 1.0 - SizeManager::GetSingleton().GetEnchantmentBonus(Player);
		int Requirement = 125 * Gigantism;
		int random = rand() % Requirement;
		if (Gigantism)
		{
		ConsoleLog::GetSingleton()->Print("Requirement: %s, Gigantism: %s", Requirement, Gigantism);
		}
		int decide_chance = 1;
		auto growthPerk = runtime.GrowthPerk;
		if (!growthPerk) {
			return false;
		}
		log::info("Random Growth random: {}", random);
		if (random <= decide_chance && Player->HasPerk(growthPerk)) {
			return true;
		} else {
			return false;
		}
	}

	void RestoreStats() {
		auto& runtime = Runtime::GetSingleton();
		if (!runtime.GrowthAugmentation) {
			return;
		}
		auto Player = PlayerCharacter::GetSingleton();
		if (Player->HasPerk(runtime.GrowthAugmentation)) {
			float HP = Player->GetPermanentActorValue(ActorValue::kHealth) * 0.00085;
			float MP = Player->GetPermanentActorValue(ActorValue::kMagicka) * 0.00085;
			float SP = Player->GetPermanentActorValue(ActorValue::kStamina) * 0.00085;
			Player->RestoreActorValue(ACTOR_VALUE_MODIFIER::kDamage, ActorValue::kHealth, HP * TimeScale());
			Player->RestoreActorValue(ACTOR_VALUE_MODIFIER::kDamage, ActorValue::kMagicka, SP * TimeScale());
			Player->RestoreActorValue(ACTOR_VALUE_MODIFIER::kDamage, ActorValue::kStamina, MP * TimeScale());
		}
	}
}

namespace Gts {
	RandomGrowth& RandomGrowth::GetSingleton() noexcept {
		static RandomGrowth instance;
		return instance;
	}
	void RandomGrowth::Update() {
		auto player = PlayerCharacter::GetSingleton();
		auto& runtime = Runtime::GetSingleton();

		if (!player) {
			return;
		}
		if (!player->Is3DLoaded()) {
			return;
		}

		bool hasSMT = runtime.SmallMassiveThreat ? player->HasMagicEffect(runtime.SmallMassiveThreat) : false;

		if (this->AllowGrowth == false) {
			static Timer timer = Timer(3.0); // Run every 3.0s or as soon as we can
			if (timer.ShouldRun()) {
				if (ShouldGrow()) {
					log::info("Random Growth True");
					// Start growing
					this->growth_time = 0.0;
					this->AllowGrowth = true;
					// Play sound
					auto MoanSound = runtime.MoanSound;
					auto GrowthSound = runtime.growthSound;
					GrowthTremorManager::GetSingleton().CallRumble(player, player, 6.0);
					float Volume = clamp(0.25, 2.0, get_visual_scale(player)/4);
					if (MoanSound) {
						PlaySound(MoanSound, player, 1.0, 0.0);
					}
					if (GrowthSound) {
						PlaySound(GrowthSound, player, Volume, 0.0);
					}
				}
			}

		} else if (this->AllowGrowth == true && !hasSMT) {
			// Do the growing
			float delta_time = Time::WorldTimeDelta();
			float Scale = get_visual_scale(player);
			float ProgressionMultiplier = runtime.ProgressionMultiplier ? runtime.ProgressionMultiplier->value : 1.0;
			float base_power = ((0.0025 * 60.0 * Scale) * ProgressionMultiplier);  // Put in actual power please
			RestoreStats(); // Regens Attributes if PC has perk
			mod_target_scale(player, base_power * delta_time); // Use delta_time so that the growth will be the same regardless of fps
			GrowthTremorManager::GetSingleton().CallRumble(player, player, 1.5);
			this->growth_time += delta_time;
			if (this->growth_time >= 2.0) { // Time in seconds" 160tick / 60 ticks per secong ~= 2.6s
				// End growing
				this->AllowGrowth = false;
			}
		}
	}
}
