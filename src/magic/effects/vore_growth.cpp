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
		float Scale = get_target_scale(target);
		this->ScaleOnVore = Scale; 
		this->BlockVoreMods = false;
	}

	void VoreGrowth::OnUpdate() {
		auto& runtime = Runtime::GetSingleton();
		float BASE_POWER = 0.0000210;
		auto caster = GetCaster();
		auto target = GetTarget();
		if (!caster || !target) {
			return;
		}
		if (caster == target) {
			return;
		}
		float bonus = 1.0;
		float GrowAmount = this->ScaleOnVore;
		BASE_POWER *= GrowAmount;
		if (caster->HasPerk(runtime.AdditionalAbsorption)) {
			BASE_POWER *= 2.0;
		}

		if (PlayerCharacter::GetSingleton()->HasMagicEffect(runtime.EffectSizeAmplifyPotion))
		{
			bonus = get_target_scale(caster) * 0.25 + 0.75;
		}
		//log::info("Vore Growth Actor: {}", caster->GetDisplayFullName());
		VoreAugmentations();
		Grow(caster, 0, BASE_POWER * bonus);
	}

	void VoreGrowth::OnFinish() {
		this->ScaleOnVore = 1.0;
		this->BlockVoreMods = false;
	}


	void VoreGrowth::VoreAugmentations() {
		
		auto Caster = GetCaster();
		if (!Caster) // Don't apply bonuses if caster is not player.
		{return;}
		auto& runtime = Runtime::GetSingleton();
		
		float HpRegen = Caster->GetPermanentActorValue(ActorValue::kHealth) * 0.00145;
		float SpRegen = Caster->GetPermanentActorValue(ActorValue::kStamina) * 0.007;

		if(Caster->HasPerk(runtime.VorePerkRegeneration))
		{
			Caster->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, ActorValue::kHealth, HpRegen * TimeScale());
			Caster->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, ActorValue::kStamina, SpRegen * TimeScale());
		}
		if (Caster->HasPerk(runtime.VorePerkGreed) && this->BlockVoreMods == false)  // Permamently increases random AV after eating someone
		{
			this->BlockVoreMods = true;
			int Boost = rand() % 2;
			if (Boost == 0)
			{
				Caster->ModActorValue(ActorValue::kHealth, 0.50);
			}
			else if (Boost == 1)
			{
				Caster->ModActorValue(ActorValue::kMagicka, 0.50);
			}
			else if (Boost == 2)
			{
				Caster->ModActorValue(ActorValue::kStamina, 0.50);
			}
		}
	}
}
