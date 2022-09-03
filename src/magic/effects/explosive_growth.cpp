#include "magic/effects/explosive_growth.hpp"
#include "managers/GrowthTremorManager.hpp"
#include "managers/GtsManager.hpp"
#include "magic/magic.hpp"
#include "magic/effects/common.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"
#include "util.hpp"
#include "timer.hpp"


namespace Gts {
	std::string ExplosiveGrowth::GetName() {
		return "ExplosiveGrowth";
	}

	ExplosiveGrowth::ExplosiveGrowth(ActiveEffect* effect) : Magic(effect) {
		const float GROWTH_1_POWER = 0.00075;
		const float GROWTH_2_POWER = 0.00095;
		const float GROWTH_3_POWER = 0.00115;

		auto base_spell = GetBaseEffect();
		auto& runtime = Runtime::GetSingleton();
		auto caster = GetCaster();
		if (!caster) {
			return;
		}
		if (base_spell == runtime.explosiveGrowth1) {
			this->power = GROWTH_1_POWER;
			if (caster->HasPerk(runtime.ExtraGrowthMax)) {
				this->grow_limit = 2.01; // NOLINT
				this->power *= 2.0; // NOLINT
			} else if (caster->HasPerk(runtime.ExtraGrowth)) {
				this->grow_limit = 1.67; // NOLINT
			} else {
				this->grow_limit = 1.34; // NOLINT
			}
		} else if (base_spell == runtime.explosiveGrowth2) {
			this->power = GROWTH_2_POWER;
			if (caster->HasPerk(runtime.ExtraGrowthMax)) {
				this->grow_limit = 2.34; // NOLINT
				this->power *= 2.0; // NOLINT
			} else if (caster->HasPerk(runtime.ExtraGrowth)) {
				this->grow_limit = 2.01; // NOLINT
			} else {
				this->grow_limit = 1.67; // NOLINT
			}
		} else if (base_spell == runtime.explosiveGrowth3) {
			this->power = GROWTH_3_POWER;
			if (caster->HasPerk(runtime.ExtraGrowthMax)) {
				this->grow_limit = 2.67; // NOLINT
				this->power *= 2.0; // NOLINT
			} else if (caster->HasPerk(runtime.ExtraGrowth)) {
				this->grow_limit = 2.34; // NOLINT
			} else {
				this->grow_limit = 2.01; // NOLINT
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
		float AdjustLimit = clamp(1.0, 12.0, runtime.CrushGrowthStorage->value + 1.0);
		float Gigantism = Persistent::GetSingleton().GetActorData(Actor)->gigantism_enchantment + 1.0;
		this->grow_limit *= AdjustLimit; //Affected by storage.
		this->grow_limit *= Gigantism; //Affected by Enchantment
	}

	void ExplosiveGrowth::OnUpdate() {
		Actor* caster = GetCaster();
		if (!caster) {
			return;
		}
		auto& runtime = Runtime::GetSingleton();


		auto HealthRegenPerk = runtime.HealthRegenPerk;
		float HpRegen = caster->GetPermanentActorValue(ActorValue::kHealth) * 0.00075;

		if (caster->HasPerk(HealthRegenPerk)) {
			caster->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, ActorValue::kHealth, HpRegen * TimeScale());
		}

		if (get_visual_scale(caster) >= this->grow_limit -0.01) {
			return;
		}

		if (this->timer.ShouldRun()) {
			auto GrowthSound = runtime.growthSound;
			float Volume = clamp(0.25, 2.0, get_visual_scale(caster)/2);
			PlaySound(GrowthSound, caster, Volume, 0.0);
			log::info("Timer is working");
		}


		Grow(caster, this->power, 0.0); // Grow
		GrowthTremorManager::GetSingleton().CallRumble(caster, caster, 1.0);
		log::info("Power {}, Limit {}", this->power, this->grow_limit);
		if (get_target_scale(caster) > this->grow_limit) {
			set_target_scale(caster, this->grow_limit);
			//Dispel; < - No need to dispel, we want to have effect active to gain bonuses from perks.
		}
	}
}
