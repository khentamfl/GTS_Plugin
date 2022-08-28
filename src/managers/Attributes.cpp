#include "managers/camera.hpp"
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
		Setting* setting = ini_conf->GetSetting(name);
		if (setting) {
			setting->data.f=value; // If float
			ini_conf->WriteSetting(setting);
		}
	}

	void BoostCarry(Actor* actor, float power) {
		auto actor_data = this->GetActorData(actor);
		if (!actor_data) {
			return;
		}
		float last_carry_boost = actor_data->last_carry_boost;
		const ActorValue av = ActorValue::kCarryWeight;
		float max_stamina = actor->GetPermanentActorValue(ActorValue::kStamina);
		float scale = get_visual_scale(actor);
		float base_av = actor->GetBaseActorValue(av);
		float current_av = actor->GetActorValue(av);

		float boost = (base_av + max_stamina * 0.5 -50.0) * (scale * power);

		float new_av = current_av - last_carry_boost + boost;
		actor_data->last_carry_boost = boost;
		actor->SetActorValue(av, new_av);

		log::info("Carry: Old base: {}, New base: {}", base_av, actor->GetBaseActorValue(av));
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
		if (actor_data) {
			base_speed = actor_data->base_walkspeedmult;
		} else {
			base_speed = 100.0;
		}
		if (scale > 1) {
			actor->SetActorValue(ActorValue::kSpeedMult, base_speed + ((scale - 1) * (100 * scale)));
		} else if (scale < 1) {
			actor->SetActorValue(ActorValue::kSpeedMult, base_speed * scale);
		} else {
			actor->SetActorValue(ActorValue::kSpeedMult, base_speed);
		}
	}

	void BoostHP(Actor* actor, float power) {
		auto actor_data = this->GetActorData(actor);
		if (!actor_data) {
			return;
		}
		float last_hp_boost = actor_data->last_hp_boost;
		const ActorValue av = ActorValue::kHealth;
		float scale = get_visual_scale(actor);
		float base_av = actor->GetBaseActorValue(av);
		float current_tempav = actor->healthModifiers.modifiers[ACTOR_VALUE_MODIFIERS::kTemporary];

		float boost = base_av * size * power;

		Player->healthModifiers.modifiers[ACTOR_VALUE_MODIFIERS::kTemporary] = current_tempav - last_hp_boost + boost;

		actor_data->last_hp_boost = boost;
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

}
