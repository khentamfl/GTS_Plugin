#include "magic/effects/grow_other.hpp"
#include "magic/effects/common.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"

namespace Gts {
	bool GrowOther::StartEffect(EffectSetting* effect) { // NOLINT
		auto& runtime = Runtime::GetSingleton();
		return effect == runtime.GrowAlly;
	}

	void GrowOther::OnUpdate() {
		const float BASE_POWER = 0.00180;
		const float SMT_BONUS = 2.0;
		const float CRUSH_BONUS = 0.00180;
		const float GROWTH_AMOUNT_BONUS = 1.4;
		const float DUAL_CAST_BONUS = 2.0;

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
		if (runtime.CrushGrowthRate->value >= GROWTH_AMOUNT_BONUS) {
			power += CRUSH_BONUS;
		}

		if (IsDualCasting()) {
			power *= DUAL_CAST_BONUS;
		}
		if (caster->HasMagicEffect(runtime.smallMassiveThreat)) {
			power *= SMT_BONUS;
		}

		Transfer(caster, target, power*0.5, power + 0.5);
	}
}
