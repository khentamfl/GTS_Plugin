#include "magic/effects/grow_other.h"
#include "magic/effects/common.h"
#include "magic/magic.h"
#include "scale/scale.h"
#include "data/runtime.h"

namespace Gts {
	bool GrowOther::StartEffect(EffectSetting* effect) {
		auto& runtime = Runtime::GetSingleton();
		return effect == runtime.GrowAlly;
	}

	void GrowOther::OnUpdate() {
		auto caster = GetCaster();
		if (!caster) {
			return;
		}
		auto target = GetTarget();
		if (!targer) {
			return;
		}

		auto& runtime = Runtime::GetSingleton();
		float size_limit = runtime.sizeLimit->value;
		float ProgressionMultiplier = runtime.ProgressionMultiplier->value;
		float CrushGrowthRate = runtime.CrushGrowthRate->value;
		float casterScale = get_visual_scale(caster);
		float targetScale = get_visual_scale(target);
		float GrowRate = 0.0;
		float SMTRate = 1.0;
		float DualCast = 1.0;
		if (caster->magicCasters[Actor::SlotTypes::kLeftHand]->GetIsDualCasting()) {
			DualCast = 2.0;
		}
		if (caster->HasMagicEffect(runtime.smallMassiveThreat)) {
			SMTRate = 2.0;
		}
		if (CrushGrowthRate >= 1.4) {
			GrowRate = 0.00180;
		}

		float transer_amount = (0.00180 + GrowRate) * (casterScale * 0.50 + 0.50) * targetScale * ProgressionMultiplier * SMTRate * DualCast;
		if (targetScale < size_limit) {
			mod_target_scale(target, transer_amount);
		}
		if (casterScale >= 1.0) {
			mod_target_scale(caster, -transer_amount);
		}
	}
}
