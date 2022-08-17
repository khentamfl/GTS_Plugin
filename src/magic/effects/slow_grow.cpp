#include "magic/effects/slow_grow.hpp"
#include "magic/effects/common.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"

namespace Gts {
	std::string SlowGrow::GetName() {
		return "SlowGrow";
	}

	bool SlowGrow::StartEffect(EffectSetting* effect) { // NOLINT
		auto& runtime = Runtime::GetSingleton();
		return effect == runtime.SlowGrowth;
	}

	void SlowGrow::OnUpdate() {
		const float BASE_POWER = 0.00100;
		const float DUAL_CAST_BONUS = 2.0;
		auto caster = GetCaster();
		if (!caster) {
			return;
		}
		float power = BASE_POWER;
		if (IsDualCasting()) {
			power *= DUAL_CAST_BONUS;
		}
		Grow(caster, 0.0, power);
	}
}
