#include "magic/explosive_growth.h"
#include "magic/magic.h"
#include "scale/scale.h"
#include "data/runtime.h"

namespace Gts {
	ExplosiveGrowth::ExplosiveGrowth(ActiveEffect* effect) {
		this->activeEffect = effect;
		this->baseEffect = effect->GetBaseEffect();
		auto& runtime = Runtime::GetSingleton();

		if (this->baseEffect == runtime.explosiveGrowth1) {
			this->power = 0.00480;
		} else if (this->baseEffect == runtime.explosiveGrowth2) {
			this->power = 0.00300;
		} else {
			this->power = 0.00175;
		}
	}

	bool StartEffect(EffectSetting* effect) {
		auto& runtime = Runtime::GetSingleton();
		if (effect == runtime.explosiveGrowth1 || effect == runtime.explosiveGrowth2 || effect == runtime.explosiveGrowth3) {
			return true;
		} else {
			return false;
		}
	}

	void ExplosiveGrowth::OnUpdate() {
		Actor* caster = GetCaster();
		if (!caster) return;

		float one = 2.0;
		float two = 3.0;
		float three = 4.0;
		float GrowthTick = 120.0;

		BGSPerk* extra_growth = find_form<BGSPerk>("GTS.esp|332563");
		TESGlobal* progression_multiplier_global = find_form<TESGlobal>("GTS.esp|37E46E");
		TESGlobal* SizeLimit = find_form<TESGlobal>("GTS.esp|2028B4");

		//BSSoundHandle GrowthSound = BSSoundHandle::BSSoundHandle();
		//auto audio_manager = BSAudioManager::GetSingleton();
		//BSISoundDescriptor* sound_descriptor = find_form<BSISoundDescriptor>("GTS.esp|271EF6");
		//audio_manager->BuildSoundDataFromDescriptor(GrowthSound, sound_descriptor);

		float progression_multiplier = progression_multiplier_global->value;
		float size_limit = SizeLimit->value;

		if (caster->HasPerk(extra_growth)) {
			one = 4.0;
			two = 6.0;
			three = 8.0;
		}
		log::info("Explosive Growth.cpp initialized");

		float scale = get_visual_scale(caster);
		if (scale <= size_limit && scale < one)
		{
			mod_target_scale(caster, (this->power * progression_multiplier));
		}
	}
}
