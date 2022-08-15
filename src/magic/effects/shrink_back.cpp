#include "magic/effects/shrink_back.h"
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
		if (!caster) {
			return;
		}
		auto target = GetTarget();
		if (!targer) {
			return;
		}

		auto& runtime = Runtime::GetSingleton();
		float ProgressionMultiplier = runtime.ProgressionMultiplier->value;
		float casterScale = get_visual_scale(caster);
		float transfer_amount = casterScale * 0.0050 + (0.0005 * 10 * ProgressionMultiplier);
		float natural_scale = 1.0;
		if (fabs(casterScale - natural_scale) <= transfer_amount) {
			set_target_scale(caster, natural_scale);
			Dispel();
		} else if (casterScale < natural_scale) {
			mod_target_scale(caster, transfer_amount);
		} else { // if (casterScale > natural_scale) {
			mod_target_scale(caster, -transfer_amount);
		}
	}
}
