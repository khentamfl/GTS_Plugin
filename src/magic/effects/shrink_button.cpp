#include "magic/effects/shrink_button.h"
#include "magic/effects/common.h"
#include "magic/magic.h"
#include "scale/scale.h"
#include "data/runtime.h"
#include "util.h"

namespace Gts {
	bool ShrinkButton::StartEffect(EffectSetting* effect) {
		auto& runtime = Runtime::GetSingleton();
		if (effect == runtime.ShrinkPCButton ) {
			return true;
		} else {
			return false;
		}
	}

	void ShrinkButton::OnUpdate() {
		auto caster = GetCaster();
		if (!caster) {
			return;
		}
		auto target = GetTarget();
		if (!targer) {
			return;
		}

		auto& runtime = Runtime::GetSingleton();

		float casterScale = get_visual_scale(caster);
		float StaminaMaxCheck = GetStaminaPercentage(caster);
		if (casterScale > 0.25) {
			DamageAV(caster, ActorValue::kStamina, 0.25 * (casterScale * 0.5 + 0.5) * StaminaMaxCheck);
			if (StaminaMaxCheck <= 0.05) {
				StaminaMaxCheck = 0.05;
			}
			mod_target_scale(caster, -0.0025 * casterScale * StaminaMaxCheck);
		}
	}
}
