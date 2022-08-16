#include "magic/effects/shrink_other.hpp"
#include "magic/effects/common.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"

namespace Gts {
	bool ShrinkOther::StartEffect(EffectSetting* effect) {
		auto& runtime = Runtime::GetSingleton();
		return effect == runtime.ShrinkAlly;
	}

	void ShrinkOther::OnUpdate() {
		auto caster = GetCaster();
		if (!caster) {
			return;
		}
		auto target = GetTarget();
		if (!target) {
			return;
		}

		auto& runtime = Runtime::GetSingleton();
		float ProgressionMultiplier = runtime.ProgressionMultiplier->value;
		float CrushGrowthRate = runtime.CrushGrowthRate->value;
		float casterScale = get_visual_scale(caster);
		float targetScale = get_visual_scale(target);
		float GrowRate = 0.0;
		float DualCast = 1.0;
		if (IsDualCasting()) {
			DualCast = 2.0;
		}
		if (CrushGrowthRate >= 1.4) {
			GrowRate = 0.00090;
		}

		if (targetScale > 0.25) {
			mod_target_scale(target, -(0.00180 + GrowRate) * (casterScale * 0.50 + 0.50) * targetScale * ProgressionMultiplier * DualCast * time_scale());
		}
	}
}
