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
		const float BASE_POWER = 0.00015;
		const float DUAL_CAST_BONUS = 2.0;
		auto caster = GetCaster();
		if (!caster) {
			return;
		}
		auto& runtime = Runtime::GetSingleton();
		float AlterBonus = caster->GetActorValue(ActorValue::kAlteration) * 0.00166 / 50;
		float power = BASE_POWER * AlterBonus;
		if (caster->HasMagicEffect(runtime.SlowGrowth2H))

		{power*= DUAL_CAST_BONUS;}

		Grow(caster, 0.0, power);
	}
}
