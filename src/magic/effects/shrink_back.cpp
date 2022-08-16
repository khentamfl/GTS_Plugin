#include "magic/effects/shrink_back.hpp"
#include "magic/effects/common.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"

namespace Gts {
	bool ShrinkBack::StartEffect(EffectSetting* effect) { // NOLINT
		auto& runtime = Runtime::GetSingleton();
		return effect == runtime.ShrinkBack;
	}

	void ShrinkBack::OnUpdate() {
		auto caster = GetCaster();
		if (!caster) {
			return;
		}
		auto& runtime = Runtime::GetSingleton();

		BSSoundHandle shrink_sound = BSSoundHandle::BSSoundHandle();
		auto audio_manager = BSAudioManager::GetSingleton();
		BSISoundDescriptor* sound_descriptor = runtime.shrinkSound;;
		audio_manager->BuildSoundDataFromDescriptor(shrink_sound, sound_descriptor);
		shrink_sound.Play();
		
		Revert(caster, 0.0025, 0.0010);
	}
}
