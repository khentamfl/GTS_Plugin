#include "magic/effects/slow_grow.hpp"
#include "managers/GrowthTremorManager.hpp"
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
		float AlterBonus = caster->GetActorValue(ActorValue::kAlteration) * 0.000005;
		float power = BASE_POWER + AlterBonus;
		float delta_time = *g_delta_time;
		auto GrowthTick = this->growth_tick;
		if (this->IsDual == true)
		{
		power*= DUAL_CAST_BONUS; 
		log::info("SlowGrowth is dual");
		}

		if (GrowthTick <= 0.0)
		{
			auto GrowthSound = runtime.growthSound;
			float Volume = clamp(0.25, 2.0, get_visual_scale(caster)/3);
			PlaySound(GrowthSound, caster, Volume, 0.75);
		}
		this->growth_tick +=delta_time;

		Grow(caster, 0.0, power);
		GrowthTremorManager::GetSingleton().CallRumble(caster, caster, 0.30);
	}

	void SlowGrow::OnStart() {
		if (IsDualCasting())
		{this->IsDual = true;}
		else
		{this->IsDual = false;}
	}

	void SlowGrow::OnFinish() {
		this->growth_tick = 0.0;
	}
}
