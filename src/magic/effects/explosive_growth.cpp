#include "magic/effects/explosive_growth.hpp"
#include "managers/GrowthTremorManager.hpp"
#include "magic/magic.hpp"
#include "magic/effects/common.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"
#include "util.hpp"


namespace Gts {
	std::string ExplosiveGrowth::GetName() {
		return "ExplosiveGrowth";
	}

	ExplosiveGrowth::ExplosiveGrowth(ActiveEffect* effect) : Magic(effect) {
		const float GROWTH_1_POWER = 0.00190;
		const float GROWTH_2_POWER = 0.00150;
		const float GROWTH_3_POWER = 0.00105;

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

	}

	void ExplosiveGrowth::OnUpdate() {
		Actor* caster = GetCaster();
		if (!caster) {
			return;
		}
		auto& runtime = Runtime::GetSingleton();
		float delta_time = *g_delta_time;
		auto GrowthTick = this->growth_time;

		if (GrowthTick <= 0.0)
		{
		auto GrowthSound = runtime.growthSound;
		float Volume = clamp(0.50, 2.0, get_visual_scale(caster));
		PlaySound(GrowthSound, caster, Volume, 1.0);
		}
		this->growth_time +=delta_time;

		if (GrowthTick >= 160.0)
		{this->growth_time = 0.0;} // Reset sound tick



		if (get_target_scale(caster) > this->grow_limit) {
			return;
		}

		Grow(caster, this->power, 0.0); // Grow
		CallRumble(caster, caster, 1.0);

		if (get_target_scale(caster) > this->grow_limit) {
			set_target_scale(caster, this->grow_limit);
			//Dispel; < - No need to dispel, we want to have effect active to gain bonuses from perks.
		}
	}
}
