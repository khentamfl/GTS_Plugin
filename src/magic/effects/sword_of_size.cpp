#include "managers/animation/Utils/AnimationUtils.hpp"
#include "magic/effects/sword_of_size.hpp"
#include "magic/effects/common.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"

namespace {
	float GetShrinkModifier(float value) {
		// https://www.desmos.com/calculator/ygoxbe7hjg
		float k = 0.9;
		float a = 0.0;
		float n = 0.7;
		float s = 1.0;
		float result = k*pow(s*(value-a), n);
		log::info("Shrink Result: {}, Value {}", result, value);
		if (value < 1.0) {
			return 1.0;
		}
		return value;
	}

	float Ench_AbsorbSize_GetPower(float power) {
		float power_result = 1.0;
		float reduction = GetShrinkModifier(power);
		power_result /= reduction;
		return power_result;
	}
}

namespace Gts {
	std::string SwordOfSize::GetName() {
		return "SwordOfSize";
	}

	SwordOfSize::SwordOfSize(ActiveEffect* effect) : Magic(effect) {
	}

	void SwordOfSize::OnUpdate() {
		auto caster = GetCaster();
		if (!caster) {
			return;
		}
		auto target = GetTarget();
		if (!target) {
			return;
		}
		float gain_value = 0.02;
		float base_shrink = std::clamp(this->power * 0.10 * Ench_AbsorbSize_GetPower(this->power * 0.10), 0.0f, 8.0f);
		float shrink_value = base_shrink * 3;

		// balanced around default value of 3.0 

		shrink_value *= Ench_AbsorbSize_GetPower(shrink_value);

		if (target->IsDead())  {
			shrink_value *= 3.0;
			gain_value *= 0.20;
		}

		TransferSize(caster, target, false, shrink_value, gain_value, false, ShrinkSource::magic);
	}

	void SwordOfSize::OnFinish() {
		auto Caster = GetCaster();
		auto Target = GetTarget();
		Task_TrackSizeTask(Caster, Target, "Sword");
	}
}

