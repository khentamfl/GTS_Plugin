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
		float BonusGrowth = 1.0;
		float progression_multiplier = runtime.ProgressionMultiplier->value;


		//BSSoundHandle GrowthSound = BSSoundHandle::BSSoundHandle();
		//auto audio_manager = BSAudioManager::GetSingleton();
		//BSISoundDescriptor* sound_descriptor = find_form<BSISoundDescriptor>("GTS.esp|271EF6");
		//audio_manager->BuildSoundDataFromDescriptor(GrowthSound, sound_descriptor);

		float size_limit = runtime.sizeLimit->value;

		if (caster->HasPerk(runtime.ExtraGrowthMax)) {
			one = 6.0;
			two = 8.0;
			three = 12.0;
			BonusGrowth = 2.0;
		}
		else if (caster->HasPerk(runtime.ExtraGrowth)) {
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
