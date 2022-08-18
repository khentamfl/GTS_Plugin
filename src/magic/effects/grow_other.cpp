#include "magic/effects/grow_other.hpp"
#include "magic/effects/common.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"

namespace Gts {
	std::string GrowOther::GetName() {
		return "GrowOther";
	}

	bool GrowOther::StartEffect(EffectSetting* effect) { // NOLINT
		auto& runtime = Runtime::GetSingleton();
		return effect == runtime.GrowAlly;
	}

	void GrowOther::OnUpdate() {
		const float BASE_POWER = 0.00090;
		const float BONUS = 0.00090;
		const float SMT_BONUS = 2.0;
		const float DUAL_CAST_BONUS = 2.0;
		const float CRUSH_GROWTH_CHECK = 1.4;

		auto caster = GetCaster();
		if (!caster) {
			return;
		}
		auto target = GetTarget();
		if (!target) {
			return;
		}

		auto& runtime = Runtime::GetSingleton();
		float caster_scale = get_visual_scale(caster);
		float target_scale = get_visual_scale(target);

		float power = BASE_POWER;
		
		if (runtime.CrushGrowthRate->value >= CRUSH_GROWTH_CHECK) {
			power += BONUS;
		}

		if (IsDualCasting()) {
			power *= DUAL_CAST_BONUS;
		}
		if (caster->HasMagicEffect(runtime.smallMassiveThreat)) {
			power *= SMT_BONUS;
		}
		float Gain = power + (caster_scale * 0.50 + 0.50 * target_scale);
		float Lose = power + * (target_scale * 0.25);

		Grow_Ally(caster, target, Gain, Lose);
	}
}
