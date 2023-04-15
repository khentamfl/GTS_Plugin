#include "magic/effects/shrink_other_button.hpp"

#include "magic/effects/common.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"
#include "managers/Rumble.hpp"

namespace Gts {
	std::string ShrinkOtherButton::GetName() {
		return "ShrinkOtherButton";
	}

	void ShrinkOtherButton::OnStart() {
		auto target = GetTarget();
		if (!target) {
			return;
		}
		float Volume = clamp(0.50, 1.0, get_target_scale(target));
		Runtime::PlaySound("shrinkSound", target, Volume, 0.0);
		//log::info("Shrink Other Button, actor: {}", target->GetDisplayFullName());
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

		float target_scale = get_target_scale(target);
		float vscale = get_visual_scale(target);
		float magicka = clamp(0.05, 1.0, GetMagikaPercentage(caster));

		float bonus = 1.0;
		if (Runtime::HasMagicEffect(PlayerCharacter::GetSingleton(), "EffectSizeAmplifyPotion")) {
			bonus = target_scale * 0.25 + 0.75;
		}

		if (target_scale > get_natural_scale(target)) {
			DamageAV(caster, ActorValue::kMagicka, 0.25 * (vscale * 0.25 + 0.75) * magicka * bonus * TimeScale());
			ShrinkActor(target, 0.0030 * magicka * bonus, 0.0);
			Rumble::Once("ShrinkOtherButton", target, 1.0, 0.05);
		}
	}
}
