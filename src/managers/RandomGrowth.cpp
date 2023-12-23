#include "managers/GtsSizeManager.hpp"
#include "managers/RandomGrowth.hpp"
#include "magic/effects/common.hpp"
#include "managers/GtsManager.hpp"
#include "utils/actorUtils.hpp"
#include "managers/Rumble.hpp"
#include "data/persistent.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "data/time.hpp"
#include "timer.hpp"

using namespace RE;
using namespace Gts;

namespace {
	bool ShouldGrow(Actor* actor) {
		float MultiplySlider = Runtime::GetFloat("RandomGrowthMultiplyPC");
		if (IsTeammate(actor)) {
			MultiplySlider = Runtime::GetFloat("RandomGrowthMultiplyNPC");
		}
		if (!Runtime::HasPerkTeam(actor, "RandomGrowth") || MultiplySlider == 0 || !IsFemale(actor)) {
			return false;
		}

		if (HasSMT(actor)) {
			return false; // Disallow random groth during Tiny Calamity
		}

		if (SizeManager::GetSingleton().BalancedMode() == 2.0) {
			MultiplySlider = 1.0; // Disable effect in Balance Mode, so slider is always 1.0
		}
		float Gigantism = 1.0 + SizeManager::GetSingleton().GetEnchantmentBonus(actor)/100;
		int Requirement = ((500 * MultiplySlider * SizeManager::GetSingleton().BalancedMode()) / Gigantism); // Doubles random in Balance Mode
		int random = rand() % Requirement;
		int decide_chance = 1;
		if (random <= decide_chance) {
			return true;
		} else {
			return false;
		}
	}

	void RestoreStats(Actor* actor) { // Regenerate attributes
		float HP = GetMaxAV(actor, ActorValue::kHealth) * 0.00185;
		float MP = GetMaxAV(actor, ActorValue::kMagicka) * 0.00095;
		float SP = GetMaxAV(actor, ActorValue::kStamina) * 0.00125;
		actor->AsActorValueOwner()->RestoreActorValue(ACTOR_VALUE_MODIFIER::kDamage, ActorValue::kHealth, HP * TimeScale());
		actor->AsActorValueOwner()->RestoreActorValue(ACTOR_VALUE_MODIFIER::kDamage, ActorValue::kMagicka, SP * TimeScale());
		actor->AsActorValueOwner()->RestoreActorValue(ACTOR_VALUE_MODIFIER::kDamage, ActorValue::kStamina, MP * TimeScale());
	}
}

namespace Gts {
	RandomGrowth& RandomGrowth::GetSingleton() noexcept {
		static RandomGrowth instance;
		return instance;
	}

	std::string RandomGrowth::DebugName() {
		return "RandomGrowth";
	}

	void RandomGrowth::Update() {
		static Timer GrowthTimer = Timer(1.0);
		if (!GrowthTimer.ShouldRunFrame()) {
			return; //Scan once per 1 sec
		}
		for (auto actor: find_actors()) {
			if (!actor) {
				return;
			}
			if (actor->formID == 0x14 || IsTeammate(actor)) {
				if (ShouldGrow(actor)) {
					// Calculations
					float scale = get_visual_scale(actor);
					float ProgressionMultiplier = Persistent::GetSingleton().progression_multiplier;
					int random = rand()% 79 + 1;
					float TotalPower = (100 + random)/100;
					float base_power = ((0.00750 * TotalPower * 60) * ProgressionMultiplier);  // The power of it
					float Gigantism = 1.0 + SizeManager::GetSingleton().GetEnchantmentBonus(actor)/100;
					ActorHandle gianthandle = actor->CreateRefHandle();
					std::string name = std::format("RandomGrowth_{}", actor->formID);
					// Sounds
					float Volume = clamp(0.20, 2.0, scale/4);
					Runtime::PlaySoundAtNode("MoanSound", actor, 1.0, 0.0, "NPC Head [Head]");
					Runtime::PlaySoundAtNode("xlRumbleL", actor, base_power, 0.0, "NPC COM [COM ]");
					Runtime::PlaySoundAtNode("growthSound", actor, Volume, 1.0, "NPC Pelvis [Pelv]");

					TaskManager::RunFor(name, 0.40 * TotalPower, [=](auto& progressData) {
						if (!gianthandle) {
							return false;
						}
						auto giantref = gianthandle.get().get();
						// Grow
						float delta_time = Time::WorldTimeDelta();
						update_target_scale(giantref, base_power * delta_time * Gigantism, SizeEffectType::kGrow);

						// Play sound
						GRumble::Once("RandomGrowth", giantref, 6.0, 0.05);
						RestoreStats(giantref); // Regens Attributes if PC has perk
						return true;
					});
				}
			}
		}
	}
}

