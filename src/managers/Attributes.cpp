#include "managers/camera.hpp"
#include "managers/GtsManager.hpp"
#include "managers/Attributes.hpp"
#include "scale/scale.hpp"
#include "util.hpp"
#include "data/runtime.hpp"
#include "data/persistent.hpp"
#include "data/transient.hpp"
#include "magic/effects/common.hpp"

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
		float value_av = actor->GetActorValue(av);

		float boost = 0.0;
		if (scale > 1.0) {
			boost = (base_av + max_stamina * 0.5 -50.0) * ((scale-1.0) * power);
		} else {
			// Linearly decrease carry weight
			//   at scale=0.0 we adjust by -base_av
			boost = base_av * (scale-1.0);
		};

		static bool done_exper = false;
		if (!done_exper) {
			done_exper = true;

			float temp_av;
			float perm_av;
			float damg_av;
			float another_perm_av;
			float delta = -20.0;

			log::info("Calling: SetBaseActorValue");
			base_av = actor->GetBaseActorValue(av);
			value_av = actor->GetActorValue(av);
			temp_av = actor->GetActorValueModifier(ACTOR_VALUE_MODIFIER::kTemporary, av);
			perm_av = actor->GetActorValueModifier(ACTOR_VALUE_MODIFIER::kPermanent, av);
			damg_av = actor->GetActorValueModifier(ACTOR_VALUE_MODIFIER::kDamage, av);
			another_perm_av = actor->GetPermanentActorValue(av);
			actor->SetBaseActorValue(av, base_av + delta);
			log::info(" - Carry: Old base:  {}, New base:  {}", base_av, actor->GetBaseActorValue(av));
			log::info(" - Carry: Old Value: {}, New Value: {}", value_av, actor->GetActorValue(av));
			log::info(" - Carry: Old Temp:  {}, New Temp:  {}", temp_av, actor->GetActorValueModifier(ACTOR_VALUE_MODIFIER::kTemporary, av));
			log::info(" - Carry: Old Perm:  {}, New Perm:  {}", perm_av, actor->GetActorValueModifier(ACTOR_VALUE_MODIFIER::kPermanent, av));
			log::info(" - Carry: Old Damg:  {}, New Damg:  {}", damg_av, actor->GetActorValueModifier(ACTOR_VALUE_MODIFIER::kDamage, av));
			log::info(" - Carry: Old PERM:  {}, New PERM:  {}", another_perm_av, actor->GetPermanentActorValue(av));

			log::info("Calling: ModActorValue");
			base_av = actor->GetBaseActorValue(av);
			value_av = actor->GetActorValue(av);
			temp_av = actor->GetActorValueModifier(ACTOR_VALUE_MODIFIER::kTemporary, av);
			perm_av = actor->GetActorValueModifier(ACTOR_VALUE_MODIFIER::kPermanent, av);
			damg_av = actor->GetActorValueModifier(ACTOR_VALUE_MODIFIER::kDamage, av);
			another_perm_av = actor->GetPermanentActorValue(av);
			actor->ModActorValue(av, delta);
			log::info(" - Carry: Old base:  {}, New base:  {}", base_av, actor->GetBaseActorValue(av));
			log::info(" - Carry: Old Value: {}, New Value: {}", value_av, actor->GetActorValue(av));
			log::info(" - Carry: Old Temp:  {}, New Temp:  {}", temp_av, actor->GetActorValueModifier(ACTOR_VALUE_MODIFIER::kTemporary, av));
			log::info(" - Carry: Old Perm:  {}, New Perm:  {}", perm_av, actor->GetActorValueModifier(ACTOR_VALUE_MODIFIER::kPermanent, av));
			log::info(" - Carry: Old Damg:  {}, New Damg:  {}", damg_av, actor->GetActorValueModifier(ACTOR_VALUE_MODIFIER::kDamage, av));
			log::info(" - Carry: Old PERM:  {}, New PERM:  {}", another_perm_av, actor->GetPermanentActorValue(av));

			log::info("Calling: SetActorValue");
			base_av = actor->GetBaseActorValue(av);
			value_av = actor->GetActorValue(av);
			temp_av = actor->GetActorValueModifier(ACTOR_VALUE_MODIFIER::kTemporary, av);
			perm_av = actor->GetActorValueModifier(ACTOR_VALUE_MODIFIER::kPermanent, av);
			damg_av = actor->GetActorValueModifier(ACTOR_VALUE_MODIFIER::kDamage, av);
			another_perm_av = actor->GetPermanentActorValue(av);
			actor->SetActorValue(av, value_av + delta);
			log::info(" - Carry: Old base:  {}, New base:  {}", base_av, actor->GetBaseActorValue(av));
			log::info(" - Carry: Old Value: {}, New Value: {}", value_av, actor->GetActorValue(av));
			log::info(" - Carry: Old Temp:  {}, New Temp:  {}", temp_av, actor->GetActorValueModifier(ACTOR_VALUE_MODIFIER::kTemporary, av));
			log::info(" - Carry: Old Perm:  {}, New Perm:  {}", perm_av, actor->GetActorValueModifier(ACTOR_VALUE_MODIFIER::kPermanent, av));
			log::info(" - Carry: Old Damg:  {}, New Damg:  {}", damg_av, actor->GetActorValueModifier(ACTOR_VALUE_MODIFIER::kDamage, av));
			log::info(" - Carry: Old PERM:  {}, New PERM:  {}", another_perm_av, actor->GetPermanentActorValue(av));

			log::info("Calling: RestoreActorValue[kPermanent]");
			base_av = actor->GetBaseActorValue(av);
			value_av = actor->GetActorValue(av);
			temp_av = actor->GetActorValueModifier(ACTOR_VALUE_MODIFIER::kTemporary, av);
			perm_av = actor->GetActorValueModifier(ACTOR_VALUE_MODIFIER::kPermanent, av);
			damg_av = actor->GetActorValueModifier(ACTOR_VALUE_MODIFIER::kDamage, av);
			another_perm_av = actor->GetPermanentActorValue(av);
			actor->RestoreActorValue(ACTOR_VALUE_MODIFIER::kPermanent, av, delta);
			log::info(" - Carry: Old base:  {}, New base:  {}", base_av, actor->GetBaseActorValue(av));
			log::info(" - Carry: Old Value: {}, New Value: {}", value_av, actor->GetActorValue(av));
			log::info(" - Carry: Old Temp:  {}, New Temp:  {}", temp_av, actor->GetActorValueModifier(ACTOR_VALUE_MODIFIER::kTemporary, av));
			log::info(" - Carry: Old Perm:  {}, New Perm:  {}", perm_av, actor->GetActorValueModifier(ACTOR_VALUE_MODIFIER::kPermanent, av));
			log::info(" - Carry: Old Damg:  {}, New Damg:  {}", damg_av, actor->GetActorValueModifier(ACTOR_VALUE_MODIFIER::kDamage, av));
			log::info(" - Carry: Old PERM:  {}, New PERM:  {}", another_perm_av, actor->GetPermanentActorValue(av));

			log::info("Calling: RestoreActorValue[kTemporary]");
			base_av = actor->GetBaseActorValue(av);
			value_av = actor->GetActorValue(av);
			temp_av = actor->GetActorValueModifier(ACTOR_VALUE_MODIFIER::kTemporary, av);
			perm_av = actor->GetActorValueModifier(ACTOR_VALUE_MODIFIER::kPermanent, av);
			damg_av = actor->GetActorValueModifier(ACTOR_VALUE_MODIFIER::kDamage, av);
			another_perm_av = actor->GetPermanentActorValue(av);
			actor->RestoreActorValue(ACTOR_VALUE_MODIFIER::kTemporary, av, delta);
			log::info(" - Carry: Old base:  {}, New base:  {}", base_av, actor->GetBaseActorValue(av));
			log::info(" - Carry: Old Value: {}, New Value: {}", value_av, actor->GetActorValue(av));
			log::info(" - Carry: Old Temp:  {}, New Temp:  {}", temp_av, actor->GetActorValueModifier(ACTOR_VALUE_MODIFIER::kTemporary, av));
			log::info(" - Carry: Old Perm:  {}, New Perm:  {}", perm_av, actor->GetActorValueModifier(ACTOR_VALUE_MODIFIER::kPermanent, av));
			log::info(" - Carry: Old Damg:  {}, New Damg:  {}", damg_av, actor->GetActorValueModifier(ACTOR_VALUE_MODIFIER::kDamage, av));
			log::info(" - Carry: Old PERM:  {}, New PERM:  {}", another_perm_av, actor->GetPermanentActorValue(av));

			log::info("Calling: RestoreActorValue[kDamage]");
			base_av = actor->GetBaseActorValue(av);
			value_av = actor->GetActorValue(av);
			temp_av = actor->GetActorValueModifier(ACTOR_VALUE_MODIFIER::kTemporary, av);
			perm_av = actor->GetActorValueModifier(ACTOR_VALUE_MODIFIER::kPermanent, av);
			damg_av = actor->GetActorValueModifier(ACTOR_VALUE_MODIFIER::kDamage, av);
			another_perm_av = actor->GetPermanentActorValue(av);
			actor->RestoreActorValue(ACTOR_VALUE_MODIFIER::kDamage, av, delta);
			log::info(" - Carry: Old base:  {}, New base:  {}", base_av, actor->GetBaseActorValue(av));
			log::info(" - Carry: Old Value: {}, New Value: {}", value_av, actor->GetActorValue(av));
			log::info(" - Carry: Old Temp:  {}, New Temp:  {}", temp_av, actor->GetActorValueModifier(ACTOR_VALUE_MODIFIER::kTemporary, av));
			log::info(" - Carry: Old Perm:  {}, New Perm:  {}", perm_av, actor->GetActorValueModifier(ACTOR_VALUE_MODIFIER::kPermanent, av));
			log::info(" - Carry: Old Damg:  {}, New Damg:  {}", damg_av, actor->GetActorValueModifier(ACTOR_VALUE_MODIFIER::kDamage, av));
			log::info(" - Carry: Old PERM:  {}, New PERM:  {}", another_perm_av, actor->GetPermanentActorValue(av));
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

	void BoostAttackDmg(Actor* actor, float power) {
		float scale = get_visual_scale(actor);
		actor->SetActorValue(ActorValue::kAttackDamageMult, (scale * power));
	}

	void BoostSpeedMulti(Actor* actor, float power) {
		float scale = get_visual_scale(actor);
		float base_speed;
		auto actor_data = Transient::GetSingleton().GetData(actor);
		base_speed = actor_data->base_walkspeedmult;
		if (actor->formID == 0x14) {
			base_speed = 100;
		}
		if (GtsManager::GetSingleton().GetFrameNum() % 30) {
			if (scale > 1) {
				actor->SetActorValue(ActorValue::kSpeedMult, base_speed + ((scale - 1) * (100 * scale)));
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
		auto HealthRegenPerk = runtime.HealthRegenPerk;

		float size = get_visual_scale(Player);

		float HpRegen = Player->GetPermanentActorValue(ActorValue::kHealth) * 0.0005;
		float MaxHealth = Player->GetPermanentActorValue(ActorValue::kHealth);

		if (Player->HasPerk(HealthRegenPerk) && (Player->HasMagicEffect(ExplGrowthP1)||Player->HasMagicEffect(ExplGrowthP2)|| Player->HasMagicEffect(ExplGrowthP3))) {
			Player->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, ActorValue::kHealth, HpRegen * TimeScale());
		}

		if (size > 0) {
			BoostHP(Player, bonusHPMultiplier);

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
		BoostSpeedMulti(Npc, 1.0);
	}

}
