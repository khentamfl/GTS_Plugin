#include "managers/camera.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/GtsManager.hpp"
#include "managers/Attributes.hpp"
#include "scale/scale.hpp"
#include "util.hpp"
#include "data/runtime.hpp"
#include "data/persistent.hpp"
#include "data/transient.hpp"
#include "magic/effects/common.hpp"
#include "timer.hpp"

using namespace SKSE;
using namespace RE;
using namespace REL;
using namespace Gts;

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

	void ManagePerkBonuses() {
		auto player = PlayerCharacter::GetSingleton();
		float gigantism = 1.0 + SizeManager::GetSingleton().GetEnchantmentBonus(player)/100;
		float BaseGlobalDamage = Runtime::GetFloat("TotalSizeDamage");
		float BaseSprintDamage = Runtime::GetFloat("TotalSprintDamage");
		float BaseFallDamage = Runtime::GetFloat("TotalFallDamage");
		float ExpectedGlobalDamage = 1.0;
		float ExpectedSprintDamage = 1.0;
		float ExpectedJumpDamage = 1.0;
	
		///Normal Damage
		if (Runtime::HasPerk(player, "Cruelty")) {
			ExpectedGlobalDamage += 0.35 * gigantism;
		}
		if (Runtime::HasPerk(player, "RealCruelty")) {
			ExpectedGlobalDamage += 1.0 * gigantism;
		}
		///Sprint Damage
		if (Runtime::HasPerk(player, "SprintDamageMult1")) {
			ExpectedSprintDamage += 0.5 * gigantism;
		}
		if (Runtime::HasPerk(player, "SprintDamageMult2")) {
			ExpectedSprintDamage += 3.5 * gigantism;
		}
		///Fall Damage
		if (Runtime::HasPerk(player, "MightyLegs")) {
			ExpectedFallDamage += 1.0 * gigantism;
		}
		if (BaseGlobalDamage != ExpectedGlobalDamage) {
			Runtime::SetFloat(BaseGlobalDamage, ExpectedGlobalDamage);
			log::info("Setting Global Damage: {}", ExpectedGlobalDamage);
		}
		if (BaseSprintDamage != ExpectedSprintDamage) {
			Runtime::SetFloat(BaseSprintDamage, ExpectedSprintDamage);
			log::info("Setting Sprint Damage: {}", ExpectedSprintDamage);
		}
		if (BaseFallDamage != ExpectedFallDamage) {
			Runtime::SetFloat(BaseFallDamage, ExpectedFallDamage);
			log::info("Setting Fall Damage: {}", ExpectedFallDamage);
		}
	}

	void BoostCarry(Actor* actor, float power) {
		auto actor_data = Persistent::GetSingleton().GetData(actor);
		if (!actor_data) {
			return;
		}
		float last_carry_boost = actor_data->bonus_carry;
		const ActorValue av = ActorValue::kCarryWeight;
		float max_stamina = actor->GetPermanentActorValue(ActorValue::kStamina);
		float visual_scale = get_target_scale(actor);
		float native_scale = get_natural_scale(actor);
		float scale = visual_scale;//native_scale;
		float base_av = actor->GetBaseActorValue(av);

		float boost = 0.0;
		if (scale > 1.0) {
			boost = (base_av) * ((scale-1.0) * power);
		} else {
			// Linearly decrease carry weight
			//   at scale=0.0 we adjust by -base_av
			boost = base_av * (scale-1.0);
		};
		actor->RestoreActorValue(ACTOR_VALUE_MODIFIER::kTemporary, av, boost - last_carry_boost);
		actor_data->bonus_carry = boost;
	}

	void BoostJump(Actor* actor, float power) {
		float scale = get_target_scale(actor);

		if (fabs(power) > 1e-5) { // != 0.0
			SetINIFloat("fJumpHeightMin", 76.0 + (76.0 * (scale - 1) * power));
			SetINIFloat("fJumpFallHeightMin", 600.0 + ( 600.0 * (scale - 1) * power));
		} else {
			SetINIFloat("fJumpHeightMin", 76.0);
			SetINIFloat("fJumpFallHeightMin", 600.0 + ((-scale + 1.0) * 300 * power));
		}
	}

	void BoostAttackDmg(Actor* actor, float power) {
		float scale = get_target_scale(actor);
		float bonus = scale * power;
		if (actor->formID == 0x14) {
			//log::info("Actor: {}, Scale:{}, Power: {}, Bonus: {}", actor->GetDisplayFullName(), scale, power, bonus);
		}
		actor->SetBaseActorValue(ActorValue::kAttackDamageMult, bonus);
	}

	void BoostSpeedMulti(Actor* actor, float power) {
		float scale = get_target_scale(actor);
		auto actor_data = Transient::GetSingleton().GetData(actor);
		float SMTBonus = Persistent::GetSingleton().GetData(actor)->smt_run_speed/3.0;
		float base_speed = actor_data->base_walkspeedmult;
		float bonusSpeedMax = Runtime::GetFloat("bonusSpeedMax");
		float speedEffectiveSize = (bonusSpeedMax / (100 * power)) + 1.0;
		if (speedEffectiveSize > scale) {
			speedEffectiveSize = scale;
		}

		static Timer timer = Timer(0.15); // Run every 0.5s or as soon as we can
		if (timer.ShouldRunFrame()) {
			if (scale > 1) {
				actor->SetActorValue(ActorValue::kSpeedMult, 100 + ((speedEffectiveSize - 1) * (100 * power)));
			} else if (scale < 1) {
				actor->SetActorValue(ActorValue::kSpeedMult, 100 * (scale * 0.90 +0.10));
			} else {
				actor->SetActorValue(ActorValue::kSpeedMult, 100);
			}
		}
	}

	void BoostHP(Actor* actor, float power) {
		auto actor_data = Persistent::GetSingleton().GetData(actor);
		if (!actor_data) {
			return;
		}
		float last_hp_boost = actor_data->bonus_hp;
		const ActorValue av = ActorValue::kHealth;
		float visual_scale = get_target_scale(actor);
		float native_scale = get_natural_scale(actor);
		float scale = visual_scale;///native_scale;

		float base_av = actor->GetBaseActorValue(av);
		float current_tempav = actor->healthModifiers.modifiers[ACTOR_VALUE_MODIFIERS::kTemporary];

		float boost;
		if (scale > 1.0) {
			boost = base_av * (scale - 1.0) * power;
		} else {
			// Linearly decrease such that:
			//   boost = -base_av when scale==0.0
			//   This way we shouldn't kill them by scaling them
			//   to zero
			boost = base_av * (scale - 1.0);
		}

		float current_health_percentage = GetHealthPercentage(actor);

		actor->healthModifiers.modifiers[ACTOR_VALUE_MODIFIERS::kTemporary] = current_tempav - last_hp_boost + boost;

		actor_data->bonus_hp = boost;

		SetHealthPercentage(actor, current_health_percentage);
		// Fill up the new healthbar
	}

	void Augmentation(Actor* Player, bool& BlockMessage) {
		auto ActorAttributes = Persistent::GetSingleton().GetData(Player);
		float Gigantism = 1.0 + SizeManager::GetSingleton().GetEnchantmentBonus(Player)/100;
		if (Player->IsSprinting() && Runtime::HasPerk(Player, "NoSpeedLoss") && Runtime::HasMagicEffect(Player, "SmallMassiveThreat")) {
			ActorAttributes->smt_run_speed += 0.001480 * Gigantism;
			if (ActorAttributes->smt_run_speed < 1.0) {
				BlockMessage = false;
			}
		} else if (Player->IsSprinting() && Runtime::HasMagicEffect(Player, "SmallMassiveThreat")) {
			ActorAttributes->smt_run_speed += 0.000960 * Gigantism;
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
		auto sizemanager = SizeManager::GetSingleton();

		float BalancedMode = SizeManager::GetSingleton().BalancedMode();

		float bonusHPMultiplier = Runtime::GetFloat("bonusHPMultiplier");
		float bonusCarryWeightMultiplier = Runtime::GetFloat("bonusCarryWeightMultiplier");
		float bonusJumpHeightMultiplier = Runtime::GetFloat("bonusJumpHeightMultiplier");
		float bonusDamageMultiplier = Runtime::GetFloat("bonusDamageMultiplier");
		float bonusSpeedMultiplier = Runtime::GetFloat("bonusSpeedMultiplier");

		float size = get_target_scale(Player);
		
		static Timer timer = Timer(0.05);

		ManagePerkBonuses();

		if (size > 0) {

			if (!Runtime::GetBool("AllowTimeChange")) {
				BoostSpeedMulti(Player, bonusSpeedMultiplier);
			}
			if (timer.ShouldRunFrame()) {
				BoostHP(Player, bonusHPMultiplier/BalancedMode);

				Augmentation(Player, BlockMessage);

				BoostCarry(Player, bonusCarryWeightMultiplier/BalancedMode);

				BoostJump(Player, bonusJumpHeightMultiplier);

				BoostAttackDmg(Player, bonusDamageMultiplier);

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
		float size = get_target_scale(npc);
		if (timer.ShouldRunFrame()) {
			if (npc->IsPlayerTeammate() || Runtime::InFaction(npc, "FollowerFaction")) {
				BoostHP(npc, 1.0);
				BoostCarry(npc, 1.0);
			}
			if (!Runtime::HasPerk(npc, "StaggerImmunity") && size > 1.33) {
				Runtime::AddPerk(npc, "StaggerImmunity");
			} else if (size < 1.33 && Runtime::HasPerk(npc, "StaggerImmunity")) {
				Runtime::RemovePerk(npc, "StaggerImmunity");
			}
			BoostAttackDmg(npc, 1.0);
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

	void AttributeManager::OverrideBonus(float Value) {
		auto ActorAttributes = Persistent::GetSingleton().GetActorData(PlayerCharacter::GetSingleton());
		ActorAttributes->smt_run_speed = Value;
	}

}
