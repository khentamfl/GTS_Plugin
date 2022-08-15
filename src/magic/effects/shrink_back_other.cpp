#include "magic/effects/shrink_other.h"
#include "magic/effects/common.h"
#include "magic/magic.h"
#include "scale/scale.h"
#include "data/runtime.h"

namespace Gts {
	bool ShrinkBackOther::StartEffect(EffectSetting* effect) {
		auto& runtime = Runtime::GetSingleton();
		return effect == runtime.ShrinkBackNPC;
	}

	void ShrinkBackOther::OnUpdate() {
		auto caster = GetCaster();
		if (!caster) return;
		auto target = GetTarget();
		if (!targer) return;

		auto& runtime = Runtime::GetSingleton();
		float size_limit = runtime.sizeLimit->value;
		float ProgressionMultiplier = runtime.ProgressionMultiplier->value;
		float targetScale = get_visual_scale(target);
		if (targetScale < 1.0)
		{set_target_scale(target, targetScale * 1.0050 + (0.0005 * 10 * ProgressionMultiplier));}
		//else if (targetScale >= 1.01 || targetScale <=1.00)
		//{runtime.ShrinkBackNPCSpell->DispelSpell(target)}
		else if (targetScale > 1.00)
		{
			set_target_scale(target, targetScale * 0.9950 - (0.0005 * 10 * ProgressionMultiplier));
		}
	}
}
