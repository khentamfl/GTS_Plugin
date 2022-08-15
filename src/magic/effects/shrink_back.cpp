#include "magic/effects/shrink_other.h"
#include "magic/effects/common.h"
#include "magic/magic.h"
#include "scale/scale.h"
#include "data/runtime.h"

namespace Gts {
	bool ShrinkBack::StartEffect(EffectSetting* effect) {
		auto& runtime = Runtime::GetSingleton();
		return effect == runtime.ShrinkBack;
	}

	void ShrinkBack::OnUpdate() {
		auto caster = GetCaster();
		if (!caster) return;
		auto target = GetTarget();
		if (!targer) return;

		auto& runtime = Runtime::GetSingleton();
		float size_limit = runtime.sizeLimit->value;
		float ProgressionMultiplier = runtime.ProgressionMultiplier->value;
		float casterScale = get_visual_scale(caster);
		if (casterScale < 1.0)
		{set_target_scale(caster, casterScale * 1.0050 + (0.0005 * 10 * ProgressionMultiplier));}
		//else if (casterScale >= 1.01 || casterScale <=1.00)
		//{runtime.ShrinkBackSpell->DispelSpell(caster)}
		else if (casterScale > 1.00)
		{
			set_target_scale(caster, casterScale * 0.9950 - (0.0005 * 10 * ProgressionMultiplier));
		}
	}
}
