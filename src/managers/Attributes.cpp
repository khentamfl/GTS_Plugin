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
		auto* ini_conf = INISettingCollection::GetSingleton(); //GameSettingCollection::GetSingleton() <- Can't compile with it
		Setting* setting = ini_conf->GetSetting(name);
		if (setting) {
			setting->data.f=value; // If float
			ini_conf->WriteSetting(setting);
		}
	}

	void BoostCarry(Actor* actor, float power) {
		const ActorValue av = ActorValue::kCarryWeight;   // <--- This function boosts Carry Weight towards infinity
		float max_stamina = actor->GetPermanentActorValue(ActorValue::kStamina);
		float scale = get_visual_scale(actor);
		float base_av = actor->GetBaseActorValue(av);
		float current_av = actor->GetActorValue(av);

		float boost = (base_av + max_stamina * 0.5 -50.0) * (scale * power);

		float new_av = current_av + boost;
		actor->SetActorValue(av, new_av);
	}

	void BoostJump(Actor* actor, float power) {
		float scale = get_visual_scale(actor);  // <--- This function is not working because of SetINIFloat

		if (fabs(power) > 1e-5) { // != 0.0
			SetINIFloat("fJumpHeightMin", 76.0 + (76.0 * (scale - 1) * power));
			SetINIFloat("fJumpFallHeightMin", 600.0 + ( 600.0 * (scale - 1) * power));
		} else {
			SetINIFloat("fJumpHeightMin", 76.0);
			SetINIFloat("fJumpFallHeightMin", 600.0 + ((-scale + 1.0) * 300 * power));
		}
	}

	void BoostAttackDmg(Actor* actor, float power) {
		float scale = get_visual_scale(actor); // <--- Works properly
		actor->SetActorValue(ActorValue::kAttackDamageMult, (scale * power));
	}

	void BoostSpeedMulti(Actor* actor, float power) {
		float scale = get_visual_scale(actor); // <--- Seems to work properly as well
		float base_speed;
		auto actor_data = Transient::GetSingleton().GetData(actor);
		if (actor_data) {
			base_speed = actor_data->base_walkspeedmult;
		} else {
			base_speed = 100.0;
		}
		if (scale > 1) {
			actor->SetActorValue(ActorValue::kSpeedMult, base_speed * scale * power);
		} else if (scale < 1) {
			actor->SetActorValue(ActorValue::kSpeedMult, base_speed * scale);
		} else {
			actor->SetActorValue(ActorValue::kSpeedMult, base_speed);
		}
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

		float bonusHP = ((Player->GetBaseActorValue(ActorValue::kHealth) * size - Player->GetBaseActorValue(ActorValue::kHealth)) * bonusHPMultiplier);
		float HpCheck = Player->GetBaseActorValue(ActorValue::kHealth) + bonusHP;
		float TempHP = Player->healthModifiers.modifiers[ACTOR_VALUE_MODIFIERS::kTemporary];
		float TempHP2 = Player->healthModifiers.modifiers[ACTOR_VALUE_MODIFIERS::kTemporary];

		if (TempHP < TempHP2) // Ugly attempt to fix missing hp
		{Player->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, ActorValue::kHealth, 25);}

		if (bonusHPMultiplier == 0.0) {
			bonusHP = 0;
		}

		{bonusHP = ((Player->GetBaseActorValue(ActorValue::kHealth) * size - Player->GetBaseActorValue(ActorValue::kHealth)) * bonusHPMultiplier);
		 HpCheck = Player->GetBaseActorValue(ActorValue::kHealth) + bonusHP;}

		Player->healthModifiers.modifiers[ACTOR_VALUE_MODIFIERS::kTemporary] = (HpCheck - MaxHealth);//Player->ModActorValue(ActorValue::kHealth, 1 * size);



		BoostCarry(Player, bonusCarryWeightMultiplier);


		if (size > 0) {
			BoostJump(Player, bonusJumpHeightMultiplier);
		}

		if (size >= 0) {
			BoostAttackDmg(Player, bonusDamageMultiplier);
		}


		if (AllowTimeChange == 0.00) {
			BoostSpeedMulti(Player, bonusSpeedMultiplier);
		}
	}

}
