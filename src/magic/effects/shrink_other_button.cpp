#include "magic/effects/shrink_other_button.hpp"
#include "magic/effects/common.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"

namespace Gts {
	bool ShrinkOtherButton::StartEffect(EffectSetting* effect) {
		auto& runtime = Runtime::GetSingleton();
		return effect == runtime.ShrinkAllySizeButton;
	}

	void ShrinkOtherButton::OnUpdate() {
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
		float MagickaMaxCheck = GetMagickaPercentage(caster);
		if (MagickaMaxCheck <= 0.05)
		{MagickaMaxCheck = 0.05;}
		if (targetScale > 1.0) {
			mod_target_scale(target, -(0.0025 * targetScale * ProgressionMultiplier));
		}
	}
}
