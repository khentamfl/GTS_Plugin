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
		if (!caster) return;
		auto target = GetTarget();
		if (!targer) return;

		auto& runtime = Runtime::GetSingleton();
		float size_limit = runtime.sizeLimit->value;
		float ProgressionMultiplier = runtime.ProgressionMultiplier->value;
		float CrushGrowthRate = runtime.CrushGrowthRate->value;
		float casterScale = get_visual_scale(caster);
		float targetScale = get_visual_scale(target);
		float GrowRate = 0.0;
		float SMTRate = 1.0;
		float DualCast = 1.0;
		if (caster->magicCasters[Actor::SlotTypes::kLeftHand]->GetIsDualCasting())
		{DualCast = 2.0;}
		if (caster->HasMagicEffect(runtime.smallMassiveThreat))
		{SMTRate = 2.0;}
		if (CrushGrowthRate >= 1.4)
		{GrowRate = 0.00180;}

		if (targetScale < size_limit) {
			set_target_scale(target, targetScale * 1.00000 + (((0.00180 + GrowRate) * (casterScale * 0.50 + 0.50) * targetScale) * ProgressionMultiplier * SMTRate * DualCast));
		}
		if (casterScale >= 1.0)
		{set_target_scale(caster, casterScale * 1.00000 - (((0.00180 + GrowRate) * targetScale * 0.50)) * ProgressionMultiplier * SMTRate * DualCast);}
	}
}
