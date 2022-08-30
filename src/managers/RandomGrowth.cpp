#include "managers/RandomGrowth.hpp"
#include "managers/GtsManager.hpp"
#include "util.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"

using namespace RE;
using namespace Gts;

namespace {
	bool ShouldGrow() {
		auto& runtime = Runtime::GetSingleton();
		int random = rand() % 5000 + 1;
		int decide_chance = 1;
		auto GrowthPerk = runtime.GrowthPerk;
		auto Player = PlayerCharacter::GetSingleton();
		if (random <= decide_chance && Player->HasPerk(GrowthPerk)) {
			return true;
		} else {
			return false;
		}
	}

	void RestoreStats() {
		auto& runtime = Runtime::GetSingleton();
		auto Player = PlayerCharacter::GetSingleton();
		if (!Player->HasPerk(runtime.GrowthAugmentation))
		{return;}
		else
		float HpRegen = Player->GetPermanentActorValue(ActorValue::kHealth) * 0.00085;
		float MpRegen = Player->GetPermanentActorValue(ActorValue::kMagicka) * 0.00085;
		float SpRegen = Player->GetPermanentActorValue(ActorValue::kStamina) * 0.00085;
		Player->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, ActorValue::kHealth, HpRegen * TimeScale());
		Player->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, ActorValue::kMagicka, MpRegen * TimeScale());
		Player->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, ActorValue::kStamina, SpRegen * TimeScale());
	}
}

namespace Gts {
	RandomGrowth& RandomGrowth::GetSingleton() noexcept {
		static RandomGrowth instance;
		return instance;
	}

	void RandomGrowth::Update() {
		auto player = PlayerCharacter::GetSingleton();
		if (!player) {
			return;
		}
		if (!player->Is3DLoaded()) {
			return;
		}

		if (this->AllowGrowth == false) {
				if (ShouldGrow()) {
					// Start growing state
					this->growth_time = 0.0;
					this->AllowGrowth = true;
					// Play sound
					auto& runtime = Runtime::GetSingleton();
					auto MoanSound = runtime.MoanSound;
					auto GrowthSound = runtime.growthSound;
					float Volume = clamp(0.25, 2.0, get_visual_scale(player)/4);
					PlaySound(MoanSound, player, 1.0);
					PlaySound(GrowthSound, player, Volume);
				}
			}
		else if (this->AllowGrowth == true && player->HasMagicEffect(runtime.SmallMassiveThreat) == false) {
				// Do the growing
				auto& runtime = Runtime::GetSingleton();
				float delta_time = *g_delta_time;
				float Scale = get_visual_scale(player);
				float ProgressionMultiplier = runtime.ProgressionMultiplier->value;
				float base_power = ((0.0025 * 60.0 * Scale) * ProgressionMultiplier);  // Put in actual power please
				RestoreStats(); // Regens Attributes if PC has perk
				mod_target_scale(player, base_power * delta_time); // Use delta_time so that the growth will be the same regardless of fps
				shake_camera(player, 0.25, 1.0);
				shake_controller(0.25, 0.25, 1.0);
				this->growth_time += delta_time;
				if (this->growth_time >= 2.0) { // Time in seconds" 160tick / 60 ticks per secong ~= 2.6s
					// End growing
					this->AllowGrowth = false;
				}
			}
		}
	}

