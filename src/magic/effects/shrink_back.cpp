#include "magic/effects/shrink_back.hpp"
#include "magic/effects/common.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"

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
		if (!target) {
			return;
		}

		auto& runtime = Runtime::GetSingleton();
		float ProgressionMultiplier = runtime.ProgressionMultiplier->value;
		float casterScale = get_visual_scale(caster);
		float transfer_amount = casterScale * 0.0025 + (0.0001 * 10 * ProgressionMultiplier);
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
