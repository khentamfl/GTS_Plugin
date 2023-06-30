#include "managers/GtsSizeManager.hpp"
#include "managers/GtsManager.hpp"
#include "managers/Attributes.hpp"
#include "managers/highheel.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"
#include "data/persistent.hpp"
#include "magic/effects/common.hpp"
#include "timer.hpp"

using namespace SKSE;
using namespace RE;
using namespace REL;
using namespace Gts;

// TODO move away from polling

namespace {
	void SetINIFloat(std::string_view name, float value) {
		auto ini_conf = GameSettingCollection::GetSingleton();
		std::string s_name(name);
		Setting* setting = ini_conf->GetSetting(s_name.c_str());
		if (setting) {
			setting->data.f=value; // If float
			ini_conf->WriteSetting(setting);
		}
	}

	bool HasGrowthPerk(Actor* actor) {
		if (!Runtime::HasPerkTeam(actor, "GrowthOfStrength")) {
			return false;
		}
		if (Runtime::HasMagicEffect(actor, "explosiveGrowth1")||Runtime::HasMagicEffect(actor, "explosiveGrowth2")||Runtime::HasMagicEffect(actor, "explosiveGrowth3")) {
			return true;
		}
		return false;
	}

	void ManagePerkBonuses(Actor* actor) {
		auto& SizeManager = SizeManager::GetSingleton();
		float BalancedMode = SizeManager::GetSingleton().BalancedMode();
		float gigantism = 1.0 + SizeManager::GetSingleton().GetEnchantmentBonus(actor)/100;

		float BaseGlobalDamage = SizeManager::GetSingleton().GetSizeAttribute(actor, 0);
		float BaseSprintDamage = SizeManager::GetSingleton().GetSizeAttribute(actor, 1);
		float BaseFallDamage = SizeManager::GetSingleton().GetSizeAttribute(actor, 2);

		float ExpectedGlobalDamage = 1.0;
		float ExpectedSprintDamage = 1.0;
		float ExpectedFallDamage = 1.0;
		float HighHeels = 1.0 + (HighHeelManager::GetSingleton().GetBaseHHOffset(actor).Length()/100);

		//log::info("High Heels Length: {}", HighHeels);
		///Normal Damage
		if (Runtime::HasPerkTeam(actor, "Cruelty")) {
			ExpectedGlobalDamage += 0.35/BalancedMode;
		}
		if (Runtime::HasPerkTeam(actor, "RealCruelty")) {
			ExpectedGlobalDamage += 0.65/BalancedMode;
		}
		if (HasGrowthPerk(actor)) {
			ExpectedGlobalDamage *= (1.0 + (0.35/BalancedMode));
		}

		ExpectedGlobalDamage *= HighHeels; // Apply Base HH damage.

		///Sprint Damage
		if (Runtime::HasPerkTeam(actor, "SprintDamageMult1")) {
			ExpectedSprintDamage += 0.25/BalancedMode;
		}
		if (Runtime::HasPerkTeam(actor, "SprintDamageMult2")) {
			ExpectedSprintDamage += 1.0/BalancedMode;
		}
		///Fall Damage
		if (Runtime::HasPerkTeam(actor, "MightyLegs")) {
			ExpectedFallDamage += 0.5/BalancedMode;
		}
		///Buff by enchantment
		ExpectedGlobalDamage *= gigantism;
		ExpectedSprintDamage *= gigantism;
		ExpectedFallDamage *= gigantism;

		if (BaseGlobalDamage != ExpectedGlobalDamage) {
			SizeManager.SetSizeAttribute(actor, ExpectedGlobalDamage, 0);
		}
		if (BaseSprintDamage != ExpectedSprintDamage) {
			SizeManager.SetSizeAttribute(actor, ExpectedSprintDamage, 1);
		}
		if (BaseFallDamage != ExpectedFallDamage) {
			SizeManager.SetSizeAttribute(actor, ExpectedFallDamage, 2);
		}
	}

	void BoostJump(Actor* actor) {
		// TODO: Write a real hook inside skyrims GETINI FLOAT
		if (actor->formID != 0x14) {
			return;
		}
		float power = AttributeManager::GetSingleton().GetAttributeBonus(actor, ActorValue::kJumpingBonus) -1.0;
		float scale = get_giantess_scale(actor);
		if (scale <= 0) {
			scale = 1.0;
		}
		if (power <= 0) {
			power = 0;
		}
		SetINIFloat("fJumpHeightMin", 76.0 + (76.0 * power));
		SetINIFloat("fJumpFallHeightMin", 600.0 + (600.0 * power));
	}

	void Augmentation(Actor* Player, bool& BlockMessage) { // Manages SMT bonus speed
		// TODO: Calc on demand rather than poll
		auto ActorAttributes = Persistent::GetSingleton().GetData(Player);
		float Gigantism = 1.0 + SizeManager::GetSingleton().GetEnchantmentBonus(Player)/100;
		if (Player->AsActorState()->IsSprinting() && Runtime::HasPerk(Player, "NoSpeedLoss") && Runtime::HasMagicEffect(Player, "SmallMassiveThreat")) {
			ActorAttributes->smt_run_speed += 0.005400 * Gigantism;
			if (ActorAttributes->smt_run_speed < 1.0) {
				BlockMessage = false;
			}
		} else if (Player->AsActorState()->IsSprinting() && Runtime::HasMagicEffect(Player, "SmallMassiveThreat")) {
			ActorAttributes->smt_run_speed += 0.003600 * Gigantism;
			if (ActorAttributes->smt_run_speed < 1.0) {
				BlockMessage = false;
			}
		} else {
			if (ActorAttributes->smt_run_speed > 0.0) {
				ActorAttributes->smt_run_speed -= 0.045000;
			} else if (ActorAttributes->smt_run_speed <= 0.0) {
				ActorAttributes->smt_run_speed -= 0.0;
				BlockMessage = false;
			} else if (ActorAttributes->smt_run_speed > 1.0) {
				ActorAttributes->smt_run_speed = 1.0;
			} else if (ActorAttributes->smt_run_speed < 1.0) {
				BlockMessage = false;
			} else {
				ActorAttributes->smt_run_speed = 0.0;
				BlockMessage = false;
			}
		}
		if (ActorAttributes->smt_run_speed >= 1.0 && !BlockMessage) {
			BlockMessage = true; // Avoid spamming it
			DebugNotification("You're fast enough to instantly crush someone", 0, true);
		}
		//log::info("SMT Bonus: {}", ActorAttributes->smt_run_speed);
	}
	// Todo unify the functions
	void UpdateActors(Actor* actor, bool& BlockMessage) {
		if (!actor) {
			return;
		}
		static Timer timer = Timer(0.05);
		static Timer jumptimer = Timer (0.20);
		float size = get_giantess_scale(actor);

		if (jumptimer.ShouldRunFrame()) {
			BoostJump(actor);
		}
		if (timer.ShouldRunFrame()) { // Run once per 0.05 sec
			ManagePerkBonuses(actor);

			if (actor->formID == 0x14) {
				Augmentation(actor, BlockMessage); // Manages SMT bonuses
			}
		}
	}
}


namespace Gts {
	AttributeManager& AttributeManager::GetSingleton() noexcept {
		static AttributeManager instance;
		return instance;
	}

	std::string AttributeManager::DebugName() {
		return "AttributeManager";
	}

	void AttributeManager::Update() {
		auto pc = PlayerCharacter::GetSingleton();
		auto healthEff = Runtime::GetMagicEffect("HealthBoost");
		for (auto actor: find_actors()) {
			UpdateActors(actor, this->BlockMessage);
		}
	}


	void AttributeManager::OverrideSMTBonus(float Value) {
		auto ActorAttributes = Persistent::GetSingleton().GetData(PlayerCharacter::GetSingleton());
		if (ActorAttributes) {
			ActorAttributes->smt_run_speed = Value;
		}
	}

	float AttributeManager::GetAttributeBonus(Actor* actor, ActorValue av) {
		if (!actor) {
			return 1.0;
		}

		float BalancedMode = SizeManager::GetSingleton().BalancedMode();
		float scale = get_giantess_scale(actor);
		if (scale <= 0) {
			scale = 1.0;
		}
		switch (av) {
			case ActorValue::kHealth: {
				float bonusHPMultiplier = Runtime::GetFloatOr("bonusHPMultiplier", 1.0);
				float power = (bonusHPMultiplier/BalancedMode);
				if (scale > 1.0) {
					return power*scale + 1.0 - power;
				} else {
					return scale;
				}
			}
			case ActorValue::kCarryWeight: {
				float bonusCarryWeightMultiplier = Runtime::GetFloatOr("bonusCarryWeightMultiplier", 1.0);
				float power = (bonusCarryWeightMultiplier/BalancedMode);
				if (scale > 1.0) {
					return power*scale + 1.0 - power;
				} else {
					return scale;
				}
			}
			case ActorValue::kSpeedMult: {
				// TODO: Rework to something more succient that garuentees 1xspeed@1xscale
				SoftPotential& MS_adjustment = Persistent::GetSingleton().MS_adjustment;
				scale = get_visual_scale(actor); // take real scale into account for MS
				float MS_mult = soft_core(scale, MS_adjustment);
				float MS_mult_limit = clamp(0.750, 1.0, MS_mult);
				float Multy = clamp(0.70, 1.0, MS_mult);
				float speed_mult_walk = soft_core(scale, this->speed_adjustment_walk);
				float bonusspeed = clamp(0.90, 1.0, speed_mult_walk);
				float PerkSpeed = 1.0;
				auto actorData = Persistent::GetSingleton().GetData(actor);
				float Bonus = 1.0;
				float animSpeed = 1.0;
				if (actorData) {
					Bonus = actorData->smt_run_speed;
					animSpeed = actorData->anim_speed;
				}

				if (Runtime::HasPerk(actor, "BonusSpeedPerk")) {
					PerkSpeed = clamp(0.80, 1.0, speed_mult_walk);
				}

				float power = 1.0 * (Bonus/2.2 + 1.0)/animSpeed;///MS_mult/MS_mult_limit/Multy/bonusspeed/PerkSpeed;
				if (scale > 1.0) {
					return power;
				} else {
					return scale;
				}
			}
			case ActorValue::kAttackDamageMult: {
				if (Runtime::HasMagicEffect(actor, "SmallMassiveThreat")) {
					scale += 3.0;
				}
				float bonusDamageMultiplier = Runtime::GetFloatOr("bonusDamageMultiplier", 1.0);
				float damage_storage = 1.0 + ((bonusDamageMultiplier) * scale - 1.0);
				if (scale > 1.0) {
					return damage_storage;
				} else {
					return scale;
				}
			}
			case ActorValue::kJumpingBonus: {
				float power = Runtime::GetFloat("bonusJumpHeightMultiplier");
				float defaultjump = 1.0 + (1.0 * (scale - 1) * power);
				if (scale > 1.0) {
					return defaultjump;
				} else {
					return scale;
				}
			}
			default: {
				return 1.0;
			}
		}
	}

	float AttributeManager::AlterGetAv(Actor* actor, ActorValue av, float originalValue) {
		float bonus = 1.0;

		auto& attributes = AttributeManager::GetSingleton();
		switch (av) {
			case ActorValue::kCarryWeight: {
				bonus = attributes.GetAttributeBonus(actor, av);
				auto transient = Transient::GetSingleton().GetData(actor);
				if (transient != nullptr) {
					transient->carryweight_boost = (originalValue * bonus) - originalValue;
				}
				break;
			}
			case ActorValue::kMovementNoiseMult: {
				float scale = get_giantess_scale(actor);
				if (scale <= 0) {
					scale = 1.0;
				}
				bonus = scale;
				break;
			}
		}

		return originalValue * bonus;
	}

	float AttributeManager::AlterGetBaseAv(Actor* actor, ActorValue av, float originalValue) {
		float finalValue = originalValue;

		switch (av) {
			case ActorValue::kHealth: {
				float bonus = 1.0;
				auto& attributes = AttributeManager::GetSingleton();
				float scale = get_giantess_scale(actor);
				if (scale <= 0) {
					scale = 1.0;
				}

				if (scale > 1.0) {
					bonus = attributes.GetAttributeBonus(actor, av);
				} else {
					//Linearly decrease such that:
					//at zero scale health=0.0
					bonus = scale;
				}

				float tempav = actor->GetActorValueModifier(ACTOR_VALUE_MODIFIER::kTemporary, av); // Do temp boosts here too
				float permav = actor->GetActorValueModifier(ACTOR_VALUE_MODIFIER::kPermanent, av);  //Do perm boosts here too
				finalValue = originalValue * bonus + (bonus - 1.0)*(tempav + permav);

				//if (actor->formID == 0x14) {
				//log::info("Health originalValue: {}", originalValue);
				//log::info("Health tempav: {}", tempav);
				//log::info("Health permav: {}", permav);
				//log::info("Health bonus: {}", bonus);
				//log::info("Health finalValue: {}", finalValue);
				auto transient = Transient::GetSingleton().GetData(actor);
				if (transient) {
					transient->health_boost = finalValue - originalValue;
				}
				//}
			}
		}

		return finalValue;
	}

	float AttributeManager::AlterSetBaseAv(Actor* actor, ActorValue av, float originalValue) {
		float finalValue = originalValue;

		switch (av) {
			case ActorValue::kHealth: {
				auto transient = Transient::GetSingleton().GetData(actor);
				if (transient) {
					float lastEdit = transient->health_boost;
					if (finalValue - lastEdit > 0.0) {
						finalValue -= lastEdit;
					}
				}
			}
		}

		return finalValue;
	}

	float AttributeManager::AlterGetPermenantAv(Actor* actor, ActorValue av, float originalValue) {
		return originalValue;
	}

	float AttributeManager::AlterGetAvMod(float originalValue, Actor* actor, ACTOR_VALUE_MODIFIER a_modifier, ActorValue a_value) {
		return originalValue;
	}

	float AttributeManager::AlterMovementSpeed(Actor* actor, const NiPoint3& direction) {
    // if (IsBeingHeld(actor)) {
    //   return 0.0;
    // }

		float bonus = 1.0;
		if (actor) {
			auto& attributes = AttributeManager::GetSingleton();
			bonus = attributes.GetAttributeBonus(actor, ActorValue::kSpeedMult);
			float volume = 0.0;
			float origSpeed = direction.Length();
			if (actor->formID != 0x14) {
				float sizedifference = get_giantess_scale(actor)/get_giantess_scale(PlayerCharacter::GetSingleton());
				volume = bonus * origSpeed * sizedifference / 250;
			} else {
				volume = bonus * origSpeed / 250;
			}
		}
		return bonus;
	}
}
