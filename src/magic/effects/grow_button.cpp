#include "magic/effects/grow_button.hpp"
#include "magic/effects/common.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"
#include "managers/Rumble.hpp"

namespace Gts {
	std::string GrowButton::GetName() {
		return "GrowButton";
	}

	void GrowButton::OnStart() {
		Actor* caster = GetCaster();
		if (!caster) {
			return;
		}
		float Volume = clamp(0.50, 2.0, get_visual_scale(caster));
		Runtime::PlaySoundAtNode("growthSound", caster, Volume, 1.0, "NPC Pelvis [Pelv]");

		//log::info("Grow Button actor: {}", caster->GetDisplayFullName());

	}

	void GrowButton::OnUpdate() {
		auto caster = GetCaster();
		if (!caster) {
			return;
		}

		float caster_scale = get_visual_scale(caster);
		float stamina = clamp(0.05, 1.0, GetStaminaPercentage(caster));
		DamageAV(caster, ActorValue::kStamina, 0.45 * (caster_scale * 0.5 + 0.5) * stamina * TimeScale());
		Grow(caster, 0.0030 * stamina, 0.0);
		Rumble::Once("GrowButton", caster, 1.0, 0.05);
	}
}
