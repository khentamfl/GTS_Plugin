#include "magic/effects/grow_other_button.hpp"
#include "magic/effects/common.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"
#include "util.hpp"

namespace Gts {
	bool GrowOtherButton::StartEffect(EffectSetting* effect) { // NOLINT
		auto& runtime = Runtime::GetSingleton();

		BSSoundHandle growth_sound = BSSoundHandle::BSSoundHandle();
		auto audio_manager = BSAudioManager::GetSingleton();
		BSISoundDescriptor* sound_descriptor = runtime.growthSound;;
		audio_manager->BuildSoundDataFromDescriptor(growth_sound, sound_descriptor);
		growth_sound.Play();

		return effect == runtime.GrowAllySizeButton;
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
		auto& runtime = Runtime::GetSingleton();


		float target_scale = get_visual_scale(target);
		float magicka = clamp(0.05, 1.0, GetMagikaPercentage(caster));

		DamageAV(caster, ActorValue::kMagicka, 0.45 * (target_scale * 0.25 + 0.75) * magicka * TimeScale());
		Grow(target, 0.0025* magicka, 0.0);
	}
}
