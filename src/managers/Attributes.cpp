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
		if (!Runtime::HasPerk(actor, "GrowthOfStrength")) {
			return false;
		}
		if (Runtime::HasMagicEffect(actor, "explosiveGrowth1")||Runtime::HasMagicEffect(actor, "explosiveGrowth2")||Runtime::HasMagicEffect(actor, "explosiveGrowth3")) {
			return true;
		}
		return false;
	}

	void ManagePerkBonuses(Actor* actor) {
		auto player = PlayerCharacter::GetSingleton();
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
		if (Runtime::HasPerk(actor, "Cruelty")) {
			ExpectedGlobalDamage += 0.35/BalancedMode;
		}
		if (Runtime::HasPerk(actor, "RealCruelty")) {
			ExpectedGlobalDamage += 0.65/BalancedMode;
		}
		if (HasGrowthPerk(actor)) {
			ExpectedGlobalDamage *= (1.0 + (0.35/BalancedMode));
		}

		ExpectedGlobalDamage *= HighHeels; // Apply Base HH damage.

		///Sprint Damage
		if (Runtime::HasPerk(actor, "SprintDamageMult1")) {
			ExpectedSprintDamage += 0.25/BalancedMode;
		}
		if (Runtime::HasPerk(actor, "SprintDamageMult2")) {
			ExpectedSprintDamage += 1.0/BalancedMode;
		}
		///Fall Damage
		if (Runtime::HasPerk(actor, "MightyLegs")) {
			ExpectedFallDamage += 0.5/BalancedMode;
		}
		///Buff by enchantment
		ExpectedGlobalDamage *= gigantism;
		ExpectedSprintDamage *= gigantism;
		ExpectedFallDamage *= gigantism;

		if (BaseGlobalDamage != ExpectedGlobalDamage) {
			SizeManager.SetSizeAttribute(actor, ExpectedGlobalDamage, 0);
			log::info("SizeManager Normal Actor {} value: {}, expected Value: {}", actor->GetDisplayFullName(), SizeManager.GetSizeAttribute(actor, 0), ExpectedGlobalDamage);
			//log::info("Setting Global Damage: {}, gigantism: {}", ExpectedGlobalDamage, gigantism);
		}
		if (BaseSprintDamage != ExpectedSprintDamage) {
			SizeManager.SetSizeAttribute(actor, ExpectedSprintDamage, 1);
			log::info("SizeManager Sprint Actor {} value: {}, expected Value: {}", actor->GetDisplayFullName(), SizeManager.GetSizeAttribute(actor, 1), ExpectedSprintDamage);
			//log::info("Setting Sprint Damage: {}, gigantism: {}", ExpectedSprintDamage, gigantism);
		}
		if (BaseFallDamage != ExpectedFallDamage) {
			SizeManager.SetSizeAttribute(actor, ExpectedFallDamage, 2);
			log::info("SizeManager Fall Actor {} value: {}, expected Value: {}", actor->GetDisplayFullName(), SizeManager.GetSizeAttribute(actor, 2), ExpectedFallDamage);
			//log::info("Setting Fall Damage: {}, gigantism: {}", ExpectedFallDamage, gigantism);
		}
	}

	void BoostJump(Actor* actor, float power) {
		float scale = get_visual_scale(actor);

		if (fabs(power) > 1e-5) { // != 0.0
			SetINIFloat("fJumpHeightMin", 76.0 + (76.0 * (scale - 1) * power));
			SetINIFloat("fJumpFallHeightMin", 600.0 + ( 600.0 * (scale - 1) * power));
		} else {
			SetINIFloat("fJumpHeightMin", 76.0);
			SetINIFloat("fJumpFallHeightMin", 600.0 + ((-scale + 1.0) * 300 * power));
		}
	}


	void Augmentation(Actor* Player, bool& BlockMessage) {
		// TODO: Calc on demand rather than poll
		auto ActorAttributes = Persistent::GetSingleton().GetData(Player);
		float Gigantism = 1.0 + SizeManager::GetSingleton().GetEnchantmentBonus(Player)/100;
		if (Player->IsSprinting() && Runtime::HasPerk(Player, "NoSpeedLoss") && Runtime::HasMagicEffect(Player, "SmallMassiveThreat")) {
			ActorAttributes->smt_run_speed += 0.003800 * Gigantism;
			if (ActorAttributes->smt_run_speed < 1.0) {
				BlockMessage = false;
			}
		} else if (Player->IsSprinting() && Runtime::HasMagicEffect(Player, "SmallMassiveThreat")) {
			ActorAttributes->smt_run_speed += 0.002600 * Gigantism;
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

	void UpdatePlayer(Actor* Player, bool& BlockMessage) {
		// Reapply Player Only

		if (!Player) {
			return;
		}
		if (!Player->Is3DLoaded()) {
			return;
		}

		float bonusJumpHeightMultiplier = Runtime::GetFloat("bonusJumpHeightMultiplier");

		float size = get_visual_scale(Player);

		static Timer timer = Timer(0.05);

		ManagePerkBonuses(Player);

		if (size > 0) {
			if (timer.ShouldRunFrame()) {
				Augmentation(Player, BlockMessage);

				BoostJump(Player, bonusJumpHeightMultiplier);

				if (!Runtime::HasPerk(Player, "StaggerImmunity") && size > 1.33) {
					Runtime::AddPerk(Player, "StaggerImmunity");
					return;
				} else if (size < 1.33 && Runtime::HasPerk(Player, "StaggerImmunity")) {
					Runtime::RemovePerk(Player, "StaggerImmunity");
				}
			}
		}
	}

	// Todo unify the functions
	void UpdateNPC(Actor* npc) {
		if (!npc) {
			return;
		}
		if (npc->formID == 0x14) {
			return;
		}
		if (!npc->Is3DLoaded()) {
			return;
		}
		static Timer timer = Timer(0.05);
		float size = get_visual_scale(npc);

		if (timer.ShouldRunFrame()) {
			if (npc->IsPlayerTeammate() || Runtime::InFaction(npc, "FollowerFaction")) {
				ManagePerkBonuses(npc);
			}
			if (!Runtime::HasPerk(npc, "StaggerImmunity") && size > 1.33) {
				Runtime::AddPerk(npc, "StaggerImmunity");
			} else if (size < 1.33 && Runtime::HasPerk(npc, "StaggerImmunity")) {
				Runtime::RemovePerk(npc, "StaggerImmunity");
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
		for (auto actor: find_actors()) {
			if (actor->formID == 0x14) {
				UpdatePlayer(actor, this->BlockMessage);
			} else {
				UpdateNPC(actor);
			}
		}
	}

	void AttributeManager::OverrideSMTBonus(float Value) {
		auto ActorAttributes = Persistent::GetSingleton().GetActorData(PlayerCharacter::GetSingleton());
		ActorAttributes->smt_run_speed = Value;
	}

	float AttributeManager::GetAttributeBonus(Actor* actor, ActorValue av) {
		if (!actor) {
			return 1.0;
		}

		float BalancedMode = SizeManager::GetSingleton().BalancedMode();
		float scale = get_visual_scale(actor);

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
				float MS_mult = soft_core(scale, MS_adjustment);
				float MS_mult_limit = clamp(0.750, 1.0, MS_mult);
				float Multy = clamp(0.70, 1.0, MS_mult);
				float speed_mult_walk = soft_core(scale, this->speed_adjustment_walk);
				float bonusspeed = clamp(0.90, 1.0, speed_mult_walk);
				float PerkSpeed = 1.0;
				float Bonus = Persistent::GetSingleton().GetActorData(actor)->smt_run_speed;
				if (Runtime::HasPerk(actor, "BonusSpeedPerk")) {
					PerkSpeed = clamp(0.80, 1.0, speed_mult_walk);
				}

				float power = 1.0 * (Bonus/2.2 + 1.0)/MS_mult/MS_mult_limit/Multy/bonusspeed/PerkSpeed;

				if (actor->formID == 0x14) {
					log::info("SpeedMult: {}", power);
				}

				return power;
			}
			case ActorValue::kAttackDamageMult: {
				float bonusDamageMultiplier = Runtime::GetFloatOr("bonusDamageMultiplier", 1.0);
				float damage_storage = scale + ((bonusDamageMultiplier) * scale - 1.0);
				return damage_storage;
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
				break;
			}
			case ActorValue::kAttackDamageMult: {
				bonus = attributes.GetAttributeBonus(actor, av);
				break;
			}
		}

		return originalValue * bonus;
	}
	float AttributeManager::AlterGetBaseAv(Actor* actor, ActorValue av, float originalValue) {
		float bonus = 1.0;
		float modav = actor->GetActorValueModifier(ACTOR_VALUE_MODIFIER::kTemporary, av); // Do temp boosts here too

		auto& attributes = AttributeManager::GetSingleton();
		switch (av) {
			case ActorValue::kHealth: {
				modav = actor->healthModifiers.modifiers[ACTOR_VALUE_MODIFIERS::kTemporary];

				float scale = get_visual_scale(actor);

				if (scale > 1.0) {
					bonus = attributes.GetAttributeBonus(actor, av);
				} else {
					// Linearly decrease such that:
					//   at zero scale health=0.0
					bonus = scale;
				}
				break;
			}
			case ActorValue::kJumpingBonus: {
				bonus = 100.0;
				break;
			}
		}

		return originalValue * bonus + (bonus - 1.0)*modav;
	}
	float AttributeManager::AlterGetPermenantAv(Actor* actor, ActorValue av, float originalValue) {
		float bonus = 1.0;
		return originalValue * bonus;
	}

	float AttributeManager::AlterMovementSpeed(Actor* actor) {
		float bonus = 1.0;
		static Timer soundtimer = Timer(0.80);
		if (actor) {
			auto& attributes = AttributeManager::GetSingleton();
			bonus = attributes.GetAttributeBonus(actor, ActorValue::kSpeedMult);
			float volume = 0.0;
			if (actor->formID != 0x14) {
				float sizedifference = get_visual_scale(actor)/get_visual_scale(PlayerCharacter::GetSingleton());
				volume = bonus * sizedifference / 250;
			} else {
				volume = bonus / 250;
			}
			if (soundtimer.ShouldRunFrame()) {
				Runtime::PlaySound("RumbleWalkSound", actor, volume, 1.0);
			}
		}
		return bonus;
	}
}
