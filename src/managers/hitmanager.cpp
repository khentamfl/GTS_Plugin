#include "managers/hitmanager.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/GrowthTremorManager.hpp"
#include "magic/effects/common.hpp"
#include "managers/GtsManager.hpp"
#include "data/persistent.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "data/time.hpp"
#include "timer.hpp"
#include "util.hpp"
#include "node.hpp"

using namespace RE;
using namespace Gts;


namespace Gts {
	HitManager& HitManager::GetSingleton() noexcept {
		static HitManager instance;
		return instance;
	}

	std::string HitManager::DebugName() {
		return "HitManager";
	}

	void HitManager::HitEvent(const TESHitEvent* a_event) {
		if (!a_event) {
			return;
		}
		auto attacker_PTR = a_event->cause;
		auto atacker_ref = attacker_PTR.get();
		auto attacker = skyrim_cast<Actor*>(atacker_ref);
		if (!attacker) {
			return;
		}
		auto receiver_PTR = a_event->target;
		auto receiver_ref = receiver_PTR.get();
		auto receiver = skyrim_cast<Actor*>(receiver_ref);
		if (!receiver) {
			return;
		}

		auto sizemanager = SizeManager::GetSingleton();
		auto& Persist = Persistent::GetSingleton();

		auto HitIdForm = a_event->source;
		auto HitId = TESForm::LookupByID(HitIdForm);

		if (HitId->GetName() == "Stagger" || HitId->GetName() == "SizeEffect" || HitId->GetName() == "SprintingSizeEffect" || HitId->GetName() == "GtsTastyFoe") {
			return;
		}

		auto ProjectileIDForm = a_event->projectile;
		auto ProjectileID = TESForm::LookupByID(ProjectileIDForm);
		auto player = PlayerCharacter::GetSingleton();

		bool wasPowerAttack = a_event->flags.all(TESHitEvent::Flag::kPowerAttack);
		bool wasSneakAttack = a_event->flags.all(TESHitEvent::Flag::kSneakAttack);
		bool wasBashAttack = a_event->flags.all(TESHitEvent::Flag::kBashAttack);
		bool wasHitBlocked = a_event->flags.all(TESHitEvent::Flag::kHitBlocked);
		static Timer timer = Timer(0.25);

		// Apply it
		if (attacker == player && Runtime::HasMagicEffect(player, "SmallMassiveThreat") && receiver != player) {
			float attackerscale = get_visual_scale(attacker) + 3.0; 
			float receiverscale = get_visual_scale(receiver); 
			float size_difference = attackerscale/receiverscale;
			if (wasPowerAttack) {
				size_difference *= 3.0;
			}
			Runtime::PlaySound("GiantImpactSound", receiver, size_difference/4, 0.0);
			GrowthTremorManager::GetSingleton().CallRumble(attacker, receiver, size_difference * 2);
			PushActorAway(receiver, size_difference);
		}

		if (receiver == player && Runtime::HasPerk(receiver, "GrowthOnHitPerk") && sizemanager.GetHitGrowth(receiver) >= 1.0 && !this->CanGrow && !this->BlockEffect) {
			if(!wasHitBlocked && !attacker->IsPlayerTeammate() && attacker != player) {
				this->BlockEffect = true;
				this->CanGrow = true;
				if (wasPowerAttack) {
					this->BonusPower = 2.0;
				} else if (!wasPowerAttack) {
					this->BonusPower = 1.0;
				}

				float SizeHunger = 1.0 + sizemanager.GetSizeHungerBonus(receiver)/100;
				float Gigantism = 1.0 + sizemanager.GetEnchantmentBonus(receiver)/100;

				float LaughChance = rand() % 12;
				float ShrinkChance = rand() % 11;

				float ReceiverScale = get_visual_scale(receiver);
				float DealerScale = get_visual_scale(attacker);

				float BalanceMode = sizemanager.BalancedMode();

				float HealthMult = GetMaxAV(receiver, ActorValue::kHealth) / receiver->GetActorValue(ActorValue::kHealth);
				float HealthPercentage = clamp(0.10, 0.50, GetHealthPercentage(receiver));

				float GrowthValue = (0.0000245 / HealthPercentage * SizeHunger * Gigantism) * this->BonusPower / sizemanager.BalancedMode();

				float SizeDifference = ReceiverScale/DealerScale;


				float clampduration = (1.0 * HealthPercentage);

				auto actor_data = Persist.GetData(receiver);
				actor_data->half_life = clampduration;

				Runtime::PlaySound("growthSound", receiver, GrowthValue * 900, 1.0);

				this->GrowthTick +=HealthPercentage;

				log::info("Clamp Duration is: {}, GrowthTicks: {}, Hit Name: {}, HitForm: {}", clampduration, this->GrowthTick, HitId->GetName(), HitIdForm);

				if (ShrinkChance >= 10) {
					mod_target_scale(attacker, (-0.025 * SizeHunger * Gigantism) * SizeDifference / BalanceMode); // Shrink Attacker
					mod_target_scale(receiver, (0.025 * SizeHunger * Gigantism) / BalanceMode); // Grow Attacker
					log::info("Shrinking Actor: {}", attacker->GetDisplayFullName());
				}

				if (SizeDifference >= 4.0 && LaughChance >= 11.0) {
					Runtime::PlaySound("LaughSound", receiver, 1.0, 0.0);
				}
				return;
			}
		} else if (receiver == player && sizemanager.BalancedMode() >= 2.0 && !this->Balance_CanShrink && !this->BlockEffect && !Runtime::HasPerk(receiver, "GrowthOnHitPerk")) {
			if(!wasHitBlocked && !attacker->IsPlayerTeammate() && attacker != player) { // If BalanceMode is 2, shrink player on hit
				this->BlockEffect = true;
				this->Balance_CanShrink = true;
				if (wasPowerAttack) {
					this->BonusPower = 2.0;
				} else if (!wasPowerAttack) {
					this->BonusPower = 1.0;
				}

				float ReceiverScale = get_visual_scale(receiver);
				float DealerScale = get_visual_scale(attacker);
				float SizeDifference = ReceiverScale/DealerScale;
				float HealthPercentage = clamp(0.10, 0.50, GetHealthPercentage(receiver));


				if (Runtime::HasMagicEffect(receiver, "EffectGrowthPotion")) {
					this->AdjustValue *= 0.50; // 50% resistance from Growth Potion.
				}
				if (Runtime::HasMagicEffect(receiver, "explosiveGrowth1") || Runtime::HasMagicEffect(receiver, "explosiveGrowth2") || Runtime::HasMagicEffect(receiver, "explosiveGrowth3")) {
					this->AdjustValue *= 0.40; // Growth Spurt 60% resistance.
				}
				if (Runtime::HasMagicEffect(receiver, "ResistShrinkPotion")) {
					this->AdjustValue *= 0.25; // 75% resistance from potion.
				}

				auto actor_data = Persist.GetData(receiver);
				float clampduration = (1.0 * HealthPercentage);
				actor_data->half_life = clampduration;

				this->GrowthTick +=HealthPercentage;

				log::info("Clamp Duration is: {}, GrowthTicks: {}, Hit Name: {}, HitId Form: {}", clampduration, this->GrowthTick, HitId->GetName(), HitIdForm);
				return;
			}
		}
	}

	void HitManager::Update() {
		auto actor = PlayerCharacter::GetSingleton();
		auto sizemanager = SizeManager::GetSingleton();
		auto& Persist = Persistent::GetSingleton();

		if (this->CanGrow) { // Grow on hit
			float SizeHunger = 1.0 + sizemanager.GetSizeHungerBonus(actor)/100;
			float Gigantism = 1.0 + sizemanager.GetEnchantmentBonus(actor)/100;
			float HealthPercentage = clamp(0.02, 1.0, GetHealthPercentage(actor));
			float GrowthValue = (0.0000245 / HealthPercentage * SizeHunger * Gigantism) * this->BonusPower / sizemanager.BalancedMode();
			float ShakePower = 1.0;

			if (IsFirstPerson()) {
				ShakePower = 0.02;
			}

			auto actor_data = Persist.GetData(actor);
			//log::info("SizeHunger, {}, Gigantism: {}", SizeHunger, Gigantism);

			if (Runtime::HasMagicEffect(actor, "SmallMassiveThreat")) {
				GrowthValue *= 0.50;
			}
			if (this->GrowthTick > 0.01 && GrowthValue > 0) {
				GrowthTremorManager::GetSingleton().CallRumble(actor, actor, (actor_data->half_life * 4) * ShakePower);
				mod_target_scale(actor, GrowthValue);
				this->GrowthTick -= 0.0005 * TimeScale();
			} else if (this->GrowthTick <= 0.01) {
				actor_data->half_life = 1.0;
				this->CanGrow = false;
				this->BlockEffect = false;
				this->GrowthTick = 0.0;
			} else {
				this->GrowthTick = 0.0;
			}
		} else if (this->Balance_CanShrink) { // Shrink on hit
			if (get_target_scale(actor) > 1.00) {
				float SizeHunger = 1.0 - sizemanager.GetSizeHungerBonus(actor)/100;
				float Gigantism = 1.0 - sizemanager.GetEnchantmentBonus(actor)/100;
				auto actor_data = Persist.GetData(actor);
				float HealthPercentage = clamp(0.05, 1.0, GetHealthPercentage(actor));
				float ShrinkValue = -(0.000085/HealthPercentage) * (get_visual_scale(actor) * 0.08 + 0.92) * SizeHunger * Gigantism * this->AdjustValue * this->BonusPower;
				float ShakePower = 1.0;

				if (IsFirstPerson()) {
					ShakePower = 0.02;
				}

				if (this->GrowthTick > 0.01 && ShrinkValue < 0) {
					GrowthTremorManager::GetSingleton().CallRumble(actor, actor, (actor_data->half_life * 2.5) * ShakePower);
					mod_target_scale(actor, ShrinkValue);
					this->GrowthTick -= 0.0005 * TimeScale();
				} else if (this->GrowthTick <= 0.01) {
					actor_data->half_life = 1.0;
					this->Balance_CanShrink = false;
					this->BlockEffect = false;
					this->GrowthTick = 0.0;
					this->AdjustValue = 1.0;

					//log::info("Shrink Value is: {}, SizeHunger: {}, Gigantism: {}", ShrinkValue, SizeHunger, Gigantism);
				} else {
					this->GrowthTick = 0.0;
				}
			}
		}
	}
}
