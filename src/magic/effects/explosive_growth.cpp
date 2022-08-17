#include "magic/effects/explosive_growth.hpp"
#include "magic/magic.hpp"
#include "magic/effects/common.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"
#include "util.hpp"


namespace Gts {
	ExplosiveGrowth::ExplosiveGrowth(ActiveEffect* effect) : Magic(effect) {
		const float GROWTH_1_POWER = 0.00480;
		const float GROWTH_2_POWER = 0.00300;
		const float GROWTH_3_POWER = 0.00175;

		auto base_spell = GetBaseEffect();
		auto& runtime = Runtime::GetSingleton();
		auto caster = GetCaster();
		if (!caster) {
			return;
		}
		if (base_spell == runtime.explosiveGrowth1) {
			this->power = GROWTH_1_POWER;
			if (caster->HasPerk(runtime.ExtraGrowthMax)) {
				this->grow_limit = 6.0; // NOLINT
				this->power *= 2.0; // NOLINT
			} else if (caster->HasPerk(runtime.ExtraGrowth)) {
				this->grow_limit = 4.0; // NOLINT
			} else {
				this->grow_limit = 2.0; // NOLINT
			}
		} else if (base_spell == runtime.explosiveGrowth2) {
			this->power = GROWTH_2_POWER;
			if (caster->HasPerk(runtime.ExtraGrowthMax)) {
				this->grow_limit = 8.0; // NOLINT
				this->power *= 2.0; // NOLINT
			} else if (caster->HasPerk(runtime.ExtraGrowth)) {
				this->grow_limit = 6.0; // NOLINT
			} else {
				this->grow_limit = 3.0; // NOLINT
			}
		} else {
			this->power = GROWTH_3_POWER;
			if (caster->HasPerk(runtime.ExtraGrowthMax)) {
				this->grow_limit = 12.0; // NOLINT
				this->power *= 2.0; // NOLINT
			} else if (caster->HasPerk(runtime.ExtraGrowth)) {
				this->grow_limit = 8.0; // NOLINT
			} else {
				this->grow_limit = 4.0; // NOLINT
			}
		}
	}

	bool ExplosiveGrowth::StartEffect(EffectSetting* effect) { // NOLINT
		auto& runtime = Runtime::GetSingleton();
		return (effect == runtime.explosiveGrowth1 || effect == runtime.explosiveGrowth2 || effect == runtime.explosiveGrowth3);
	}

	void ExplosiveGrowth::OnStart() {
		Actor* caster = GetCaster();
		if (!caster) {
			return;
		}
		auto& runtime = Runtime::GetSingleton();

		//BSSoundHandle growth_sound = BSSoundHandle::BSSoundHandle();
		//auto audio_manager = BSAudioManager::GetSingleton();
		//BSISoundDescriptor* sound_descriptor = runtime.growthSound;
		//audio_manager->BuildSoundDataFromDescriptor(growth_sound, sound_descriptor);
		//growth_sound.Play();
		
		shake_controller(this->power * 0.5, this->power * 0.5, 1.0);
	}

	void ExplosiveGrowth::OnUpdate() {
		Actor* caster = GetCaster();
		if (!caster) {
			return;
		}
		auto& runtime = Runtime::GetSingleton();




		if (get_target_scale(caster) > this->grow_limit) {
			//Dispel; < - No need to dispel, we want to have effect active to gain bonuses from perks.
			return;
		}

		Grow(caster, this->power, 0.0);
		if (get_target_scale(caster) > this->grow_limit) {
			set_target_scale(caster, this->grow_limit);
			shake_camera(caster, this->power * 0.5, 1.0);
			//Dispel; < - No need to dispel, we want to have effect active to gain bonuses from perks.
		}
	}
}
