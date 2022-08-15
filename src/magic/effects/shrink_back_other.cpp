#include "magic/effects/shrink_back_other.hpp"
#include "magic/effects/common.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"

namespace Gts {
	bool ShrinkBackOther::StartEffect(EffectSetting* effect) {
		auto& runtime = Runtime::GetSingleton();
		return effect == runtime.ShrinkBackNPC;
	}

	void ShrinkBackOther::OnUpdate() {
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
		float targetScale = get_visual_scale(target);
		float transfer_amount = targetScale * 0.0025 + (0.0001 * 10 * ProgressionMultiplier);
		float natural_scale = 1.0;
		if (fabs(targetScale - natural_scale) <= transfer_amount) {
			set_target_scale(target, natural_scale);
			Dispel();
		} else if (targetScale < natural_scale) {
			mod_target_scale(target, transfer_amount);
		} else { // if (casterScale > natural_scale) {
			mod_target_scale(target, -transfer_amount);
		}
	}
}
