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

	void BoostCarry(Actor* actor, float power) {
		auto actor_data = Persistent::GetSingleton().GetData(actor);
		if (!actor_data) {
			return;
		}
		float last_carry_boost = actor_data->bonus_carry;
		const ActorValue av = ActorValue::kCarryWeight;
		float max_stamina = actor->GetPermanentActorValue(ActorValue::kStamina);
		float visual_scale = get_visual_scale(actor);
		float native_scale = get_natural_scale(actor);
		float scale = visual_scale/native_scale;
		float base_av = actor->GetBaseActorValue(av);

		float boost = 0.0;
		if (scale > 1.0) {
			boost = (base_av + max_stamina * 0.5 -50.0) * ((scale-1.0) * power);
		} else {
			// Linearly decrease carry weight
			//   at scale=0.0 we adjust by -base_av
			boost = base_av * (scale-1.0);
		};
		actor->RestoreActorValue(ACTOR_VALUE_MODIFIER::kTemporary, av, boost - last_carry_boost);
		actor_data->bonus_carry = boost;
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

	void BoostAttackDmg(Actor* actor, float power) {
		float scale = get_target_scale(actor);
		float bonus = scale * power;
		actor->SetActorValue(ActorValue::kAttackDamageMult, bonus);
	}

	void BoostSpeedMulti(Actor* actor, float power) {
		auto runtime = Runtime::GetSingleton();
		float scale = get_visual_scale(actor);
		auto actor_data = Transient::GetSingleton().GetData(actor);
		float SMTBonus = Persistent::GetSingleton().GetData(actor)->smt_run_speed/3.0;
		float base_speed = actor_data->base_walkspeedmult;
		float bonusSpeedMax = runtime.bonusSpeedMax->value;
		float speedEffectiveSize = (bonusSpeedMax / (100 * power)) + 1.0;

		static Timer timer = Timer(0.15); // Run every 0.5s or as soon as we can
		if (timer.ShouldRunFrame()) {
			if (scale > 1) {
				actor->SetActorValue(ActorValue::kSpeedMult, base_speed + ((speedEffectiveSize - 1) * (100 * power)));
			} else if (scale < 1) {
				actor->SetActorValue(ActorValue::kSpeedMult, base_speed * (scale * 0.90 +0.10));
			} else {
				actor->SetActorValue(ActorValue::kSpeedMult, base_speed);
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
		float visual_scale = get_visual_scale(actor);
		float native_scale = get_natural_scale(actor);
		float scale = visual_scale/native_scale;

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
		auto& runtime = Runtime::GetSingleton();
		auto AugmentationPerk = runtime.NoSpeedLoss;
		auto ActorAttributes = Persistent::GetSingleton().GetData(Player);
		float Gigantism = 1.0 + SizeManager::GetSingleton().GetEnchantmentBonus(Player)/100;
		if (Player->IsSprinting() && Player->HasPerk(AugmentationPerk) && Player->HasMagicEffect(runtime.SmallMassiveThreat)) {
			ActorAttributes->smt_run_speed += 0.001480 * Gigantism;
			if (ActorAttributes->smt_run_speed < 1.0) {
				BlockMessage = false;
			}
		} else if (Player->IsSprinting() && Player->HasMagicEffect(runtime.SmallMassiveThreat)) {
			ActorAttributes->smt_run_speed += 0.000960 * Gigantism;
			if (ActorAttributes->smt_run_speed < 1.0) {
				BlockMessage = false;
			}
		} else {
			if (ActorAttributes->smt_run_speed > 0.0) {
				ActorAttributes->smt_run_speed -= 0.004175;
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
		auto& runtime = Runtime::GetSingleton();
		

		auto SmallMassiveThreat = runtime.SmallMassiveThreat;

		float AllowTimeChange = runtime.AllowTimeChange->value;
		float bonusHPMultiplier = runtime.bonusHPMultiplier->value;

		float bonusCarryWeightMultiplier = runtime.bonusCarryWeightMultiplier->value;
		float bonusJumpHeightMultiplier = runtime.bonusJumpHeightMultiplier->value;
		float bonusDamageMultiplier = runtime.bonusDamageMultiplier->value;
		float bonusSpeedMultiplier = runtime.bonusSpeedMultiplier->value;

		auto ExplGrowthP1 = runtime.explosiveGrowth1;
		auto ExplGrowthP2 = runtime.explosiveGrowth2;
		auto ExplGrowthP3 = runtime.explosiveGrowth3;
		

		auto CharController = Player->GetCharController();

		//float velocity = CharController->velocityMod;
		//float up = CharController->up;
		float unk180 = CharController->unk180;
		float unk188 = CharController->unk188;
		//float pad198 = CharController->pad198;
		float waterH = CharController->waterHeight;
		float swimH = CharController->swimFloatHeight;
		float ActorH = CharController->actorHeight;
		float speed = CharController->speedPct;

		static Timer timer = Timer(3.00); // Run every 0.5s or as soon as we can
		if (timer.ShouldRunFrame()) {
			//log::info("velocity: {}", velocity);
			//log::info("up: {}", up);
			log::info("unk180: {}", unk180);
			log::info("unk188: {}", unk188);
			//log::info("unk198: {}", unk198);
			log::info("waterH: {}", waterH);
			log::info("swimH: {}", swimH);
			log::info("ActorH: {}", ActorH);
			log::info("speed: {}", speed);
		}

		float size = get_target_scale(Player);

		if (size > 0) {
			BoostHP(Player, bonusHPMultiplier);

			Augmentation(Player, BlockMessage);

			BoostCarry(Player, bonusCarryWeightMultiplier);

			BoostJump(Player, bonusJumpHeightMultiplier);

			BoostAttackDmg(Player, bonusDamageMultiplier);

			if (AllowTimeChange == 0.00) {
				BoostSpeedMulti(Player, bonusSpeedMultiplier);
			}
			if (!Player->HasPerk(runtime.StaggerImmunity) && size > 1.33)
			{
				Player->AddPerk(runtime.StaggerImmunity);
				return; 
			}
			else if (size < 1.33 && Player->HasPerk(runtime.StaggerImmunity))
			{
				Player->RemovePerk(runtime.StaggerImmunity);
			}
		}
	}

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
		auto& runtime = Runtime::GetSingleton();
		float size = get_target_scale(npc);
		if (npc->IsPlayerTeammate() || npc->IsInFaction(runtime.FollowerFaction)) {
			BoostHP(npc, 1.0);
			BoostCarry(npc, 1.0);
		}
		if (!npc->HasPerk(runtime.StaggerImmunity) && size > 1.33) {
			npc->AddPerk(runtime.StaggerImmunity);
		}
		else if (size < 1.33 && npc->HasPerk(runtime.StaggerImmunity)) {
			npc->RemovePerk(runtime.StaggerImmunity);
		}
		BoostAttackDmg(npc, 1.0);
	}
}


namespace Gts {
	AttributeManager& AttributeManager::GetSingleton() noexcept {
		static AttributeManager instance;
		return instance;
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