#include "magic/effects/shrink_button.hpp"
#include "magic/effects/common.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"
#include "util.hpp"

namespace Gts {
	bool ShrinkButton::StartEffect(EffectSetting* effect) { // NOLINT
		auto& runtime = Runtime::GetSingleton();

		BSSoundHandle shrink_sound = BSSoundHandle::BSSoundHandle();
		auto audio_manager = BSAudioManager::GetSingleton();
		BSISoundDescriptor* sound_descriptor = runtime.shrinkSound;
		audio_manager->BuildSoundDataFromDescriptor(shrink_sound, sound_descriptor);
		shrink_sound.Play();

		return (effect == runtime.ShrinkPCButton);
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
			ShrinkActor(caster, 0.0025*stamina, 0.0);

		}
	}
}
