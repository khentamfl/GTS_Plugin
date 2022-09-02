#include "managers/camera.hpp"
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
		float scale = get_visual_scale(actor);
		actor->SetActorValue(ActorValue::kAttackDamageMult, (scale * power));
	}

	void BoostSpeedMulti(Actor* actor, float power) {
		float scale = get_visual_scale(actor);
		float base_speed;
		auto actor_data = Transient::GetSingleton().GetData(actor);
		float SMTBonus = Persistent::GetSingleton().GetData(actor)->smt_run_speed/3.0;
		if (actor != PlayerCharacter::GetSingleton()) {
			base_speed = actor_data->base_walkspeedmult;
		} else {
			base_speed = 100.00;
		}
		static Timer timer = Timer(0.5); // Run every 0.5s or as soon as we can
		if (timer.ShouldRun()) {
			if (scale > 1) {
				actor->SetActorValue(ActorValue::kSpeedMult, base_speed + ((scale - 1) * (100 * scale)) * (SMTBonus + 1.0));
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
}


namespace Gts {
	AttributeManager& AttributeManager::GetSingleton() noexcept {
		static AttributeManager instance;
		return instance;
	}

	void AttributeManager::Update() {
		// Reapply Player Only

		auto Player = RE::PlayerCharacter::GetSingleton();
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
		float bonusSpeedMax = runtime.bonusSpeedMax->value;

		auto ExplGrowthP1 = runtime.explosiveGrowth1;
		auto ExplGrowthP2 = runtime.explosiveGrowth2;
		auto ExplGrowthP3 = runtime.explosiveGrowth3;

		float size = get_visual_scale(Player);

		if (size > 0) {
			BoostHP(Player, bonusHPMultiplier);

			Augmentation();

			BoostCarry(Player, bonusCarryWeightMultiplier);

			BoostJump(Player, bonusJumpHeightMultiplier);

			BoostAttackDmg(Player, bonusDamageMultiplier);

			if (AllowTimeChange == 0.00) {
				BoostSpeedMulti(Player, bonusSpeedMultiplier);
			}
		}
	}


	void AttributeManager::UpdateNpc(Actor* Npc) {
		// Reapply Player Only

		if (!Npc) {
			return;
		}
		if (Npc->formID == 0x14) {
			return;
		}
		if (!Npc->Is3DLoaded()) {
			return;
		}

		BoostAttackDmg(Npc, 1.0);

		//BoostSpeedMulti(Npc, 1.0);
	}

	void AttributeManager::Augmentation() {
		auto Player = PlayerCharacter::GetSingleton();
		auto& runtime = Runtime::GetSingleton();
		auto AugmentationPerk = runtime.NoSpeedLoss;
		auto ActorAttributes = Persistent::GetSingleton().GetActorData(Player);
		if (Player->IsSprinting() && Player->HasPerk(AugmentationPerk) && Player->HasMagicEffect(runtime.SmallMassiveThreat)) {
			ActorAttributes->smt_run_speed += 0.001480;
		} else if (Player->IsSprinting() && Player->HasMagicEffect(runtime.SmallMassiveThreat)) {
			ActorAttributes->smt_run_speed += 0.000960;
		} else {
			if (ActorAttributes->smt_run_speed > 0.0) {
				ActorAttributes->smt_run_speed -= 0.004175;
			} 

			else if (ActorAttributes->smt_run_speed <= 0.0) {
				ActorAttributes->smt_run_speed -= 0.0;
				this->BlockMessage = false;
			} 
			
			else if (ActorAttributes->smt_run_speed > 1.0) {
				ActorAttributes->smt_run_speed = 1.0;
			}

			else if (ActorAttributes->smt_run_speed < 1.0) {
				this->BlockMessage = false;
			}

			else {
				ActorAttributes->smt_run_speed = 0.0;
				this->BlockMessage = false;
			}
		}
		if (ActorAttributes->smt_run_speed >= 1.0 && !this->BlockMessage) {
			this->BlockMessage = true; // Avoid spamming it
			DebugNotification("You're fast enough to instantly crush someone", 0, true);
		}
		log::info("SMT Bonus: {}", ActorAttributes->smt_run_speed);
	}

	void AttributeManager::OverrideBonus(float Value) {
		this->MovementSpeedBonus = Value;
	}

}
