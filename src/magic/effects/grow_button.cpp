#include "managers/animation/AnimationManager.hpp"
#include "magic/effects/grow_button.hpp"
#include "magic/effects/common.hpp"
#include "managers/Rumble.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"

namespace {
}

namespace Gts {
	std::string GrowButton::GetName() {
		return "GrowButton";
	}

	void GrowButton::OnStart() {
		Actor* caster = GetCaster();
		if (!caster) {
			return;
		}
		float stamina = clamp(0.05, 1.0, GetStaminaPercentage(caster));
		float Volume = clamp(0.20, 2.0, stamina * get_visual_scale(caster)/8);
		Runtime::PlaySoundAtNode("growthSound", caster, Volume, 1.0, "NPC Pelvis [Pelv]");
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
		GRumble::Once("GrowButton", caster, 1.0, 0.05);
	}
}
