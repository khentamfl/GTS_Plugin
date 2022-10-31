#include "magic/effects/shrink_other.hpp"
#include "magic/effects/common.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"

namespace Gts {
	std::string ShrinkOther::GetName() {
		return "ShrinkOther";
	}

	bool ShrinkOther::StartEffect(EffectSetting* effect) { // NOLINT
		auto& runtime = Runtime::GetSingleton();
		return (effect == runtime.ShrinkAlly || effect == runtime.ShrinkAllyAdept || effect == runtime.ShrinkAllyExpert);
	}

	void ShrinkOther::OnUpdate() {
		auto& runtime = Runtime::GetSingleton();
		const float BASE_POWER = 0.00180;
		const float CRUSH_BONUS = 0.00180;
		const float GROWTH_AMOUNT_BONUS = 1.4;
		const float DUAL_CAST_BONUS = 2.0;
		const float SCALE_FACTOR = 0.5;

		auto caster = GetCaster();
		if (!caster) {
			return;
		}
		auto target = GetTarget();
		if (!target) {
			return;
		}

		float power = BASE_POWER;

		if (base_spell == runtime.ShrinkAllyAdept) 
		{
			power *= 1.32;
		}
		else if (base_spell == runtime.ShrinkAllyExpert)
		{
			power *= 1.68;
		}

		float caster_scale = get_visual_scale(caster);
		float target_scale = get_visual_scale(target);

		if (runtime.CrushGrowthRate->value >= GROWTH_AMOUNT_BONUS) {
			power += CRUSH_BONUS;
		}

		if (IsDualCasting()) {
			power *= DUAL_CAST_BONUS;
		}

		ShrinkActor(target, power*0.5, 0);
	}
}
