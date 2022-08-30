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

		//BSSoundHandle shrink_sound = BSSoundHandle::BSSoundHandle();
		//auto audio_manager = BSAudioManager::GetSingleton();
		//BSISoundDescriptor* sound_descriptor = runtime.shrinkSound;
		//audio_manager->BuildSoundDataFromDescriptor(shrink_sound, sound_descriptor);
		//shrink_sound.Play();

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
		PlaySound(ShrinkSound, caster, Volume, 1.0);
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
		float stamina = clamp(0.25, 1.0, GetStaminaPercentage(caster));
		if (caster_scale > 0.25) {
			DamageAV(caster, ActorValue::kStamina, 0.25 * (caster_scale * 0.5 + 0.5) * stamina * TimeScale());
			ShrinkActor(caster, 0.0030*stamina, 0.0);
			GrowthTremorManager::GetSingleton().CallRumble(caster, caster, 0.60);
		}
	}
}
