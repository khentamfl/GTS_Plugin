#include "magic/effects/absorb_effect.hpp"
#include "magic/effects/common.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"

namespace Gts {
	bool Absorb::StartEffect(EffectSetting* effect) {
		auto& runtime = Runtime::GetSingleton();
		return (effect == runtime.SlowGrowth || effect == runtime.SlowGrowth)
	}

	void SlowGrow::OnUpdate() {
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
        float targetScale = get_visual_scale(target);
        float SizeDifference = casterScale/targetScale;

		mod_target_scale(caster, -0.0025 * ProgressionMultiplier * SizeDifference);
	}
}
