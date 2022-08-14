#include "magic/effects/shrink_for.h"
#include "magic/effects/common.h"
#include "magic/magic.h"
#include "scale/scale.h"
#include "data/runtime.h"

namespace Gts {
	bool GrowButton::StartEffect(EffectSetting* effect) {
		auto& runtime = Runtime::GetSingleton();
		if (effect == runtime.GrowPcButton ) {
			return true;
		} else {
			return false;
		}
	}

	void GrowButton::OnUpdate() {
		auto caster = GetCaster();
		if (!caster) return;
		auto target = GetTarget();
		if (!targer) return;

		auto& runtime = Runtime::GetSingleton();
		float size_limit = runtime.sizeLimit->value;
		float casterScale = get_visual_scale(caster);
		float StaminaMaxCheck = caster->GetActorValue(ActorValue::kStamina)/caster->GetPermanentActorValue(ActorValue::kStamina);
		float PermanentSP = caster->GetPermanentActorValue(ActorValue::kStamina);
		log::info("MaxCheck is {}", StaminaMaxCheck); log::info("Permanent SP is {}", PermanentSP);
		if (casterScale < size_limit) {
			if (StaminaMaxCheck <= 0.05)
			{StaminaMaxCheck = 0.05;}
			caster->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, ActorValue::kStamina, ((-0.45 * (casterScale * 0.5 + 0.5)) * StaminaMaxCheck));
			mod_target_scale(caster, (0.0025 * casterScale) * StaminaMaxCheck);
		}
	}
}
