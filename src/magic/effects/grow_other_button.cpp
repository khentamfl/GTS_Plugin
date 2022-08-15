#include "magic/effects/grow_other_button.hpp"
#include "magic/effects/common.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"
#include "util.hpp"

namespace Gts {
	bool GrowOtherButton::StartEffect(EffectSetting* effect) {
		auto& runtime = Runtime::GetSingleton();
		return effect == runtime.GrowAllySizeButton;
	}

	void GrowOtherButton::OnUpdate() {
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
		DamageAV(caster, ActorValue::kMagicka, 0.45 * (targetScale * 0.25 + 0.75) * MagickaMaxCheck);
		mod_target_scale(target, 0.0025 * targetScale * ProgressionMultiplier);
	}
}
