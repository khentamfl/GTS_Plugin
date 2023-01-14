#include "magic/effects/vore_growth.hpp"
#include "magic/effects/common.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"


namespace Gts {
	std::string VoreGrowth::GetName() {
		return "VoreGrowth";
	}

	void VoreGrowth::OnStart() {
		auto target = GetTarget();
		float Scale = get_target_scale(target);
		this->ScaleOnVore = Scale;
		this->BlockVoreMods = false;
	}

	void VoreGrowth::OnUpdate() {
		float BASE_POWER = 0.0000360;
		auto caster = GetCaster();
		if (!caster) {
			return;
		}
		auto target = GetTarget();
		if (!target) {
			return;
		}
		float bonus = 1.0;
		float GrowAmount = this->ScaleOnVore;
		BASE_POWER *= GrowAmount;
		if (Runtime::HasMagicEffect(caster, "AdditionalAbsorption")) {
			BASE_POWER *= 2.0;
		}

		if (Runtime::HasMagicEffect(PlayerCharacter::GetSingleton(),"EffectSizeAmplifyPotion")) {
			bonus = get_target_scale(caster) * 0.25 + 0.75;
		}
		//log::info("Vore Growth Actor: {}, Target: {}", caster->GetDisplayFullName(), target->GetDisplayFullName());
		VoreAugmentations();
		Grow(caster, 0, BASE_POWER * bonus);
	}

	void VoreGrowth::OnFinish() {
		this->ScaleOnVore = 1.0;
		this->BlockVoreMods = false;
	}


	void VoreGrowth::VoreAugmentations() {

		auto Caster = GetCaster();
		if (!Caster) { // Don't apply bonuses if caster is not player.
			return;
		} 
		auto Target = GetTarget();
		if (!Target) { // Don't apply bonuses if caster is not player.
			return;
		}

		float HpRegen = GetMaxAV(Caster, ActorValue::kHealth) * 0.00105;
		float SpRegen = GetMaxAV(Caster, ActorValue::kStamina) * 0.0020;

		if (Runtime::HasPerk(Caster, "VorePerkRegeneration")) {
			Caster->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, ActorValue::kHealth, HpRegen * TimeScale());
			Caster->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, ActorValue::kStamina, SpRegen * TimeScale());
			log::info("Player's HP Max {}, Regen: {}", Caster->GetMaxAV(ActorValue::kHealth), HpRegen);
		}
		if (Runtime::HasPerk(Caster, "VorePerkGreed") && this->BlockVoreMods == false) { // Permamently increases random AV after eating someone
			this->BlockVoreMods = true;
			float TotalMod = 0.75 * get_visual_scale(Target);
			int Boost = rand() % 2;
			if (Boost == 0) {
				Caster->ModActorValue(ActorValue::kHealth, TotalMod);
			} else if (Boost == 1) {
				Caster->ModActorValue(ActorValue::kMagicka, TotalMod);
			} else if (Boost == 2) {
				Caster->ModActorValue(ActorValue::kStamina, TotalMod);
			}
		}
	}
}
