#include "magic/effects/shrink_other.h"
#include "magic/effects/common.h"
#include "magic/magic.h"
#include "scale/scale.h"
#include "data/runtime.h"

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
		if (!targer) {
			return;
		}

		auto& runtime = Runtime::GetSingleton();
		float ProgressionMultiplier = runtime.ProgressionMultiplier->value;
		float CrushGrowthRate = runtime.CrushGrowthRate->value;
		float casterScale = get_visual_scale(caster);
		float targetScale = get_visual_scale(target);
		float GrowRate = 0.0;
		float DualCast = 1.0;
		if (IsDuelCasting()) {
			DualCast = 2.0;
		}
		if (CrushGrowthRate >= 1.4) {
			GrowRate = 0.00090;
		}

		if (targetScale > 0.25) {
			mod_target_scale(target, -(0.00180 + GrowRate) * (casterScale * 0.50 + 0.50) * targetScale * ProgressionMultiplier * DualCast);
		}
	}
}
