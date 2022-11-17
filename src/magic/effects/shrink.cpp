#include "magic/effects/shrink.hpp"
#include "magic/effects/common.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"

namespace Gts {
	std::string Shrink::GetName() {
		return "Shrink";
	}

	bool Shrink::StartEffect(EffectSetting* effect) { // NOLINT
		return effect == Runtime::GetMagicEffect("ShrinkSpell");
	}

	void Shrink::OnUpdate() {
		const float BASE_POWER = 0.00360;
		const float DUAL_CAST_BONUS = 2.0;
		auto caster = GetCaster();
		if (!caster) {
			return;
		}
		float SkillMult = 1.0 + caster->GetActorValue(ActorValue::kAlteration) / 100;

		float power = BASE_POWER * SkillMult;

		float bonus = 1.0;
		if (Runtime::HasMagicEffect(PlayerCharacter::GetSingleton(), "EffectSizeAmplifyPotion")) {
			bonus = get_target_scale(caster) * 0.25 + 0.75;
		}

		if (IsDualCasting()) {
			power *= DUAL_CAST_BONUS;
		}
		ShrinkActor(caster, 0.0, power * bonus);
	}
}
