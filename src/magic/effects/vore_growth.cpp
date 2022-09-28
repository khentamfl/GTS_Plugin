#include "magic/effects/vore_growth.hpp"
#include "magic/effects/common.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"


namespace Gts {
	std::string VoreGrowth::GetName() {
		return "VoreGrowth";
	}

	bool VoreGrowth::StartEffect(EffectSetting* effect) { // NOLINT
		auto& runtime = Runtime::GetSingleton();
		return effect == runtime.GlobalVoreGrowth;
	}

	void VoreGrowth::OnStart() {
		auto target = GetTarget();
		float Scale = get_visual_scale(target);
		this->ScaleOnVore = Scale; 
		this->BlockVoreMods = false;
	}

	void VoreGrowth::OnUpdate() {
		float BASE_POWER = 0.0005200;
		auto& runtime = Runtime::GetSingleton();
		auto caster = GetCaster();
		auto target = GetTarget();
		if (!caster || !target) {
			return;
		}
		float GrowAmount = this->ScaleOnVore;
		BASE_POWER *= GrowAmount;
		if (caster->HasPerk(runtime.AdditionalAbsorption))
		{BASE_POWER *= 2.0;}
		VoreAugmentations();
		Grow(caster, 0.0, BASE_POWER);
	}

	void VoreGrowth::OnFinish() {
		this->ScaleOnVore = 1.0;
		this->BlockVoreMods = false;
	}


	void VoreGrowth::VoreAugmentations() {
		
		auto Caster = GetCaster();
		auto Player = PlayerCharacter::GetSingleton();
		if (!Caster || Caster != Player) // Don't apply bonuses if caster is not player.
		{return;}
		auto& runtime = Runtime::GetSingleton();
		
		float HpRegen = Player->GetPermanentActorValue(ActorValue::kHealth) * 0.00145;
		float SpRegen = Player->GetPermanentActorValue(ActorValue::kStamina) * 0.007;
		float k = 0.150; 
        float n = 1.0; 
        float s = 1.12;
        float Modification = 1.0/(pow(1.0+pow(k*(get_visual_scale(Player)-1.0),n*s),1.0/s)); 
		if(Player->HasPerk(runtime.VorePerkRegeneration))
		{
			Player->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, ActorValue::kHealth, HpRegen * TimeScale());
			Player->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, ActorValue::kStamina, SpRegen * TimeScale());
		}
		if (Player->HasPerk(runtime.VorePerkGreed) && this->BlockVoreMods == false)  // Permamently increased random AV after eating someone
		{
			this->BlockVoreMods = true;
			int Boost = rand() % 2;
			if (Boost == 0)
			{
				Player->ModActorValue(ActorValue::kHealth, 0.50);
			}
			else if (Boost == 1)
			{
				Player->ModActorValue(ActorValue::kMagicka, 0.50);
			}
			else if (Boost == 2)
			{
				Player->ModActorValue(ActorValue::kStamina, 0.50);
			}
		}
	}
}
