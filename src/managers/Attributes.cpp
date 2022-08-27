#include "managers/camera.hpp"
#include "managers/Attributes.hpp"
#include "scale/scale.hpp"
#include "util.hpp"
#include "data/runtime.hpp"
#include "data/persistent.hpp"
#include "magic/effects/common.hpp"

using namespace SKSE;
using namespace RE;
using namespace REL;
using namespace Gts;

namespace {
	void SetINIFloat(std::string_view name, float value) {
		auto ini_conf = INISettingCollection::GetSingleton();
		Setting* setting = ini_conf->GetSetting(name);
		if (setting) {
			setting->data.f=value; // If float
			ini_conf->WriteSetting(setting);
		}
	}}


namespace Gts {
	AttributeManager& AttributeManager::GetSingleton() noexcept {
		static AttributeManager instance;
		return instance;
	}

void AttributeManager::Update() {
	// Reapply Player Only

		auto Player = PlayerCharacter::GetSingleton();
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

		if (Player->HasPerk(HealthRegenPerk) && (Player->HasMagicEffect(ExplGrowthP1)||Player->HasMagicEffect(ExplGrowthP2)|| Player->HasMagicEffect(ExplGrowthP3)))
    		{Player->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, ActorValue::kHealth, HpRegen * TimeScale());}

		float bonusHP = ((Player->GetBaseActorValue(ActorValue::kHealth) * size - Player->GetBaseActorValue(ActorValue::kHealth)) * bonusHPMultiplier);
        float HpCheck = Player->GetBaseActorValue(ActorValue::kHealth) + bonusHP;
        float TempHP = Player->healthModifiers.modifiers[ACTOR_VALUE_MODIFIERS::kTemporary];

		if (bonusHPMultiplier == 0.0)  {
			bonusHP = 0;
			}

		if (size < 1)    

    	{bonusHP = ((Player->GetBaseActorValue(ActorValue::kHealth) * size - Player->GetBaseActorValue(ActorValue::kHealth)) * bonusHPMultiplier);
    	HpCheck = Player->GetBaseActorValue(ActorValue::kHealth) + bonusHP;
        }
        if (MaxHealth < HpCheck)     {
			Player->healthModifiers.modifiers[ACTOR_VALUE_MODIFIERS::kTemporary] = TempHP + (HpCheck - MaxHealth);//Player->ModActorValue(ActorValue::kHealth, 1 * size);
			}

     if (MaxHealth > HpCheck + (1 * size) && MaxHealth > HpCheck) {
		Player->healthModifiers.modifiers[ACTOR_VALUE_MODIFIERS::kTemporary] = TempHP + (HpCheck - MaxHealth); Player->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, ActorValue::kHealth, 1 * size);
        //Player->ModActorValue(ActorValue::kHealth, -1 * size); 
		}

	float MaxStamina = Player->GetPermanentActorValue(ActorValue::kStamina);

	float bonusCarryWeight = ((Player->GetBaseActorValue(ActorValue::kCarryWeight) + (MaxStamina * 0.5) -50.0) * (size * bonusCarryWeightMultiplier));
    if (Player->GetBaseActorValue(ActorValue::kCarryWeight) < bonusCarryWeight && Player->GetActorValue(ActorValue::kCarryWeight) < bonusCarryWeight)
        {Player->ModActorValue(ActorValue::kCarryWeight, 1 * size);}    
    if (Player->GetActorValue(ActorValue::kCarryWeight) > bonusCarryWeight)
        {Player->ModActorValue(ActorValue::kCarryWeight, -1 * size);} 


	if (size > 0)
        if (bonusJumpHeightMultiplier != 0.0)
        {
            SetINIFloat("fJumpHeightMin", 76.0 + (76.0 * (size - 1) * bonusJumpHeightMultiplier));
            SetINIFloat("fJumpFallHeightMin", 600.0 + ( 600.0 * (size - 1) * bonusJumpHeightMultiplier));
        }
    else
        if (bonusJumpHeightMultiplier != 0.0)
        {
            SetINIFloat("fJumpHeightMin", 76.0);
            SetINIFloat("fJumpFallHeightMin", 600.0 + ((-size + 1.0) * 300 * bonusJumpHeightMultiplier));
        }	

        if (size >= 0)
    {Player->SetActorValue(ActorValue::kAttackDamageMult, (size * bonusDamageMultiplier));}

    else
    if (bonusDamageMultiplier == 0.0)
        {Player->SetActorValue(ActorValue::kAttackDamageMult, (size * bonusDamageMultiplier));}
    else
        {Player->SetActorValue(ActorValue::kAttackDamageMult, (size * bonusDamageMultiplier));}

  
       if (size > 1 && AllowTimeChange>= 1.00) {
        Player->SetActorValue(ActorValue::kSpeedMult, (100 + ((size - 1) * (100 * bonusSpeedMultiplier))));
        }   
       
       else 
        if (size < 1 && AllowTimeChange >= 1.00) {
        Player->SetActorValue(ActorValue::kSpeedMult, (100 * size));
        }

    else
        if (bonusSpeedMultiplier == 0.0 && AllowTimeChange>= 1.00)
            {Player->SetActorValue(ActorValue::kSpeedMult, 100);
        }
    }

}