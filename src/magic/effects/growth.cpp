#include "magic/effects/growth.hpp"
#include "magic/effects/common.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"
#include "util.hpp"

namespace Gts {
	std::string Growth::GetName() {
		return "Growth";
	}

	bool Growth::StartEffect(EffectSetting* effect) { // NOLINT
		auto& runtime = Runtime::GetSingleton();
		return (effect == Runtime::GetMagicEffect("GrowthSpell") || effect == Runtime::GetMagicEffect("GrowthSpellAdept") || effect == Runtime::GetMagicEffect("GrowthSpellExpert"));

	}

	void Growth::OnUpdate() {
		const float BASE_POWER = 0.00150;
		const float DUAL_CAST_BONUS = 2.0;
		auto& runtime = Runtime::GetSingleton();
		

		auto caster = GetCaster();
		if (!caster) {
			return;
		}


		float SkillMult = 1.0 + caster->GetActorValue(ActorValue::kAlteration) / 200;
		float HpRegen = caster->GetPermanentActorValue(ActorValue::kHealth) * 0.00035;

		float bonus = 1.0;

		if (Runtime::HasMagicEffect(PlayerCharacter::GetSingleton(),"EffectSizeAmplifyPotion"))
		{
			bonus = get_target_scale(caster) * 0.25 + 0.75;
		}

		float power = BASE_POWER * SkillMult;

		auto base_spell = GetBaseEffect();

		if (base_spell == Runtime::GetMagicEffect("GrowthSpellAdept")) {
			power *= 1.33;
		}
		else if (base_spell == Runtime::GetMagicEffect("GrowthSpellExpert")) {
			power *= 1.75;
			caster->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, ActorValue::kHealth, HpRegen * TimeScale());
		}


		if (IsDualCasting()) {
			power *= DUAL_CAST_BONUS;
		}

		Grow(caster, 0.0, power * bonus);
	}
}
