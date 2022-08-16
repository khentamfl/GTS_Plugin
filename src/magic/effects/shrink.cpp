#include "magic/effects/shrink.hpp"
#include "magic/effects/common.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"

namespace Gts {
	bool Shrink::StartEffect(EffectSetting* effect) { // NOLINT
		auto& runtime = Runtime::GetSingleton();
		return effect == runtime.ShrinkSpell;
	}

	void Shrink::OnUpdate() {
		const float BASE_POWER = 0.00180;
		const float DUAL_CAST_BONUS = 2.0;
		auto caster = GetCaster();
		if (!caster) {
			return;
		}

		float power = BASE_POWER;
		if (IsDualCasting()) {
			power *= DUAL_CAST_BONUS;
		}
		//Shrink(caster, power, 0.0); <- Can't get it to compile
	}
}
