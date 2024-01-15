#include "magic/effects/shrink_button.hpp"
#include "magic/effects/common.hpp"
#include "utils/actorUtils.hpp"

#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"
#include "managers/Rumble.hpp"

namespace Gts {
	std::string ShrinkButton::GetName() {
		return "ShrinkButton";
	}

	void ShrinkButton::OnStart() {
		auto caster = GetCaster();
		if (!caster) {
			return;
		}
		float Volume = clamp(0.10, 1.0, get_visual_scale(caster) * 0.1);
		Runtime::PlaySound("shrinkSound", caster, Volume, 1.0);
		//log::info("Shrink Buton Sound, actor: {}", caster->GetDisplayFullName());
	}

	void ShrinkButton::OnUpdate() {
		auto caster = GetCaster();
		if (!caster) {
			return;
		}
		auto target = GetTarget();
		if (!target) {
			return;
		}

		float caster_scale = get_visual_scale(caster);
		float target_scale = get_target_scale(caster);
		float stamina = clamp(0.05, 1.0, GetStaminaPercentage(caster));

		if (target_scale > 0.12) {
			DamageAV(caster, ActorValue::kStamina, 0.25 * (caster_scale * 0.5 + 0.5) * stamina * TimeScale());
			ShrinkActor(caster, 0.0030* stamina, 0.0);
			GRumble::Once("ShrinkButton", caster, 0.60, 0.05);
		} else {
			set_target_scale(caster, 0.12);
		}
	}
}
