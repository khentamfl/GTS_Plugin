#include "magic/effects/grow_other_button.hpp"
#include "managers/GrowthTremorManager.hpp"
#include "magic/effects/common.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"

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
		float Volume = clamp(0.50, 2.0, get_target_scale(target));
		Runtime::PlaySound("growthSound", target, Volume, 0.0);
		log::info("Grow Other Button, actor: {}", target->GetDisplayFullName());
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

		float target_scale = get_target_scale(target);
		float magicka = clamp(0.05, 1.0, GetMagikaPercentage(caster));

		float bonus = 1.0;
		if (Runtime::HasMagicEffect(PlayerCharacter::GetSingleton(), "EffectSizeAmplifyPotion")) {
			bonus = get_target_scale(target) * 0.25 + 0.75;
		}

		DamageAV(caster, ActorValue::kMagicka, 0.45 * (target_scale * 0.25 + 0.75) * magicka * bonus * TimeScale());


		Grow(target, 0.0030 * magicka * bonus, 0.0);
		GrowthTremorManager::GetSingleton().CallRumble(target, caster, 1.0);
	}
}
