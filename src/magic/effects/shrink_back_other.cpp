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
