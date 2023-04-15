#include "magic/effects/grow_other_button.hpp"

#include "magic/effects/common.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"
#include "managers/Rumble.hpp"

namespace Gts {
	std::string GrowOtherButton::GetName() {
		return "GrowOtherButton";
	}

	void GrowOtherButton::OnStart() {
		Actor* caster = GetCaster();
		if (!caster) {
			return;
		}
		Actor* target = GetTarget();
		if (!target) {
			return;
		}
		float Volume = clamp(0.50, 2.0, get_visual_scale(target));
		Runtime::PlaySound("growthSound", target, Volume, 0.0);
		//log::info("Grow Other Button, actor: {}", target->GetDisplayFullName());
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

		float scale = get_visual_scale(target);
		float magicka = clamp(0.05, 1.0, GetMagikaPercentage(caster));

		float bonus = 1.0;
		if (Runtime::HasMagicEffect(PlayerCharacter::GetSingleton(), "EffectSizeAmplifyPotion")) {
			bonus = scale * 0.25 + 0.75;
		}

		DamageAV(caster, ActorValue::kMagicka, 0.45 * (scale * 0.25 + 0.75) * magicka * bonus * TimeScale());
		Grow(target, 0.0030 * magicka * bonus, 0.0);
		Rumble::Once("GrowButton", target, 1.0, 0.05);
	}
}
