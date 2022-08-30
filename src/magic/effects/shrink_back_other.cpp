#include "magic/effects/shrink_back_other.hpp"
#include "magic/effects/common.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"

namespace Gts {
	std::string ShrinkBackOther::GetName() {
		return "ShrinkBackOther";
	}

	bool ShrinkBackOther::StartEffect(EffectSetting* effect) { // NOLINT
		auto& runtime = Runtime::GetSingleton();
		return effect == runtime.ShrinkBackNPC;
	}

	void ShrinkBackOther::OnStart() {
		Actor* target = GetTarget();
		if (!target) {
			return;
		}
		auto& runtime = Runtime::GetSingleton();
		auto ShrinkSound = runtime.shrinkSound;
		float Volume = clamp(0.50, 1.0, get_visual_scale(target));
		PlaySound(ShrinkSound, target, Volume, 1.0);
	}

	void ShrinkBackOther::OnUpdate() {
		auto target = GetTarget();
		if (!target) {
			return;
		}
		float Power = 0.0025;
		auto& runtime = Runtime::GetSingleton();

		//BSSoundHandle shrink_sound = BSSoundHandle::BSSoundHandle();
		//auto audio_manager = BSAudioManager::GetSingleton();
		//BSISoundDescriptor* sound_descriptor = runtime.shrinkSound;
		//audio_manager->BuildSoundDataFromDescriptor(shrink_sound, sound_descriptor);
		//shrink_sound.Play();
		if (DualCasted())
		{Power *= 2.0;}

		if (!Revert(target, Power, Power/2.5)) {
			// Returns false when shrink back is complete
			Dispel();
		}
	}
}
