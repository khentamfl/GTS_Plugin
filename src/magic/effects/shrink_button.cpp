#include "magic/effects/shrink_button.hpp"
#include "magic/effects/common.hpp"

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
		float Volume = clamp(0.50, 1.0, get_visual_scale(caster));
		Runtime::PlaySound("shrinkSound", caster, Volume, 0.0);
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
		float stamina = clamp(0.05, 1.0, GetStaminaPercentage(caster));

		if (caster_scale > 0.10) {
			DamageAV(caster, ActorValue::kStamina, 0.25 * (caster_scale * 0.5 + 0.5) * stamina * TimeScale());
			ShrinkActor(caster, 0.0030* stamina, 0.0);
			Rumble::Once("ShrinkButton", caster, 0.60, 0.05, 1.0);
		}
	}
}
