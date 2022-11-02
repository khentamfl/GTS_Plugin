#include "magic/effects/explosive_growth.hpp"
#include "managers/GrowthTremorManager.hpp"
#include "managers/GtsSizeManager.hpp"
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
		
	}

	bool ExplosiveGrowth::StartEffect(EffectSetting* effect) { // NOLINT
		auto& runtime = Runtime::GetSingleton();
		return (effect == runtime.explosiveGrowth1 || effect == runtime.explosiveGrowth2 || effect == runtime.explosiveGrowth3);
	}

	void ExplosiveGrowth::OnStart() {

	}

	void ExplosiveGrowth::OnUpdate() {
		Actor* caster = GetCaster();
		if (!caster) {
			return;
		}
		const float GROWTH_1_POWER = 0.00125;
		const float GROWTH_2_POWER = 0.00145;
		const float GROWTH_3_POWER = 0.00175;

		auto& runtime = Runtime::GetSingleton();
		auto base_spell = GetBaseEffect();
		
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
		
		auto sizemanager = SizeManager::GetSingleton();
		float AdjustLimit = clamp(1.0, 12.0, runtime.CrushGrowthStorage->value + 1.0);
		float Gigantism = 1.0 + sizemanager.GetEnchantmentBonus(caster)/100;
		float GetGrowthSpurt = SizeManager::GetSingleton().GetGrowthSpurt(caster);
		float scale = get_target_scale(caster);

		float bonus = 1.0;

		float limit = this->grow_limit * Gigantism * AdjustLimit;

		auto HealthRegenPerk = runtime.HealthRegenPerk;
		float HpRegen = caster->GetPermanentActorValue(ActorValue::kHealth) * 0.00080;

		if (caster->HasPerk(HealthRegenPerk)) {
			caster->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, ActorValue::kHealth, HpRegen * TimeScale());
		}
		
		if (scale <= limit || limit > GetGrowthSpurt) {
			
			if (PlayerCharacter::GetSingleton()->HasMagicEffect(runtime.EffectSizeAmplifyPotion))
			{
				bonus = get_target_scale(caster) * 0.25 + 0.75;
			}
			DoGrowth(caster, this->power * bonus);
			//this->RequiredSizeChange = 0;
			SizeManager::GetSingleton().SetGrowthSpurt(caster, limit);
		}

		//else if (limit <= GetGrowthSpurt || GetGrowthSpurt > limit) {
			//float difference = GetGrowthSpurt - limit;
			//log::info("RequiredSizeChange: {}", RequiredSizeChange);
			//if (this->RequiredSizeChange <= 0 && difference > 0.10) {
				//SizeManager::GetSingleton().SetGrowthSpurt(caster, limit);
				//this->RequiredSizeChange = difference;
			//}

			//if (this->RequiredSizeChange > 0) {
				//this->RequiredSizeChange -= difference/100;
				//DoShrink(caster, difference/100);
				//if (this->RequiredSizeChange <= 0)
				//{
					//this->RequiredSizeChange = 0;
					//return;
				//}
			//}
			//log::info("Difference is: {}", difference);
			//log::info("RequiredSizeChange: {}, Difference: {}", RequiredSizeChange, difference);
		}
		//log::info("Growth Spurt: {}, Total Limit is: {}, Gigantism: {}, CrushGrowthStorage: {}, Target Scale: {}", GetGrowthSpurt, limit, Gigantism, AdjustLimit, scale);

	void ExplosiveGrowth::OnFinish() {
		Actor* caster = GetCaster();
		if (!caster) {
			return;
		}
		SizeManager::GetSingleton().SetGrowthSpurt(caster, 0.0);
	}



	void ExplosiveGrowth::DoGrowth(Actor* actor, float value) {
			mod_target_scale(actor, value); // Grow
			auto runtime = Runtime::GetSingleton();

			GrowthTremorManager::GetSingleton().CallRumble(actor, actor, 1.0);
			if (this->timerSound.ShouldRunFrame()) {
				PlaySound(runtime.xlRumbleL, actor, this->power/20, 0.0);
			}
			if (this->timer.ShouldRun()) {
				auto GrowthSound = runtime.growthSound;
				float Volume = clamp(0.12, 2.0, get_visual_scale(actor)/4);
				PlaySound(GrowthSound, actor, Volume, 0.0);
			}
		}

		void ExplosiveGrowth::DoShrink(Actor* actor, float value) {
			mod_target_scale(actor, -value); // Grow
			auto runtime = Runtime::GetSingleton();

			GrowthTremorManager::GetSingleton().CallRumble(actor, actor, 1.0);
			if (this->timerSound.ShouldRunFrame()) {
				PlaySound(runtime.xlRumbleL, actor, this->power/20, 0.0);
			}
			if (this->timer.ShouldRun()) {
				auto GrowthSound = runtime.shrinkSound;
				float Volume = clamp(0.12, 2.0, get_visual_scale(actor)/2);
				PlaySound(GrowthSound, actor, Volume, 0.0);
			}
		}	
	}
