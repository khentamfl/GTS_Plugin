#include "magic/effects/shrink_button.hpp"
#include "magic/effects/common.hpp"
#include "managers/GrowthTremorManager.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"
#include "util.hpp"

namespace Gts {
	std::string ShrinkButton::GetName() {
		return "ShrinkButton";
	}

	bool ShrinkButton::StartEffect(EffectSetting* effect) { // NOLINT
		auto& runtime = Runtime::GetSingleton();
		return (effect == runtime.ShrinkPCButton);
	}

	void ShrinkButton::OnStart() {
		auto caster = GetCaster();
		if (!caster) {
			return;
		}
		auto& runtime = Runtime::GetSingleton();
		auto ShrinkSound = runtime.shrinkSound;
		float Volume = clamp(0.50, 1.0, get_visual_scale(caster));
		PlaySound(ShrinkSound, caster, Volume, 0.0);
		log::info("Shrink Buton Sound, actor: {}", caster->GetDisplayFullName());
	}

	void ShrinkButton::OnUpdate() {
		auto caster = GetCaster();
		auto& runtime = Runtime::GetSingleton();
		if (!caster) {
			return;
		}
		auto target = GetTarget();
		if (!target) {
			return;
		}

		float caster_scale = get_visual_scale(caster);
		float stamina = clamp(0.25, 1.0, GetStaminaPercentage(caster));
		
		auto& runtime = Runtime::GetSingleton();

		float bonus = 1.0;
		if (PlayerCharacter::GetSingleton()->HasMagicEffect(runtime.EffectSizeAmplifyPotion))
		{
			bonus = get_target_scale(caster);
		}

		if (caster_scale > 0.25) {
			DamageAV(caster, ActorValue::kStamina, 0.25 * (caster_scale * 0.5 + 0.5) * stamina * bonus * TimeScale());
			ShrinkActor(caster, 0.0030* stamina * bonus, 0.0);
			GrowthTremorManager::GetSingleton().CallRumble(caster, caster, 0.60);
		}
	}
}
