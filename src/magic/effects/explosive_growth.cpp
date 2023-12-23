#include "magic/effects/explosive_growth.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/GtsManager.hpp"
#include "magic/magic.hpp"
#include "magic/effects/common.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"
#include "timer.hpp"
#include "managers/Rumble.hpp"



namespace Gts {
	std::string ExplosiveGrowth::GetName() {
		return "ExplosiveGrowth";
	}

	ExplosiveGrowth::ExplosiveGrowth(ActiveEffect* effect) : Magic(effect) {

	}

	void ExplosiveGrowth::OnStart() {
		Actor* caster = GetCaster();
		if (!caster) {
			return;
		}
		//std::string message = std::format("Growth Spurt slowly increases your size until you reach maximum scale of growth spurt. Growth Spurt doubles amount of size that you can steal during hugs and Growth Spurt also provides huge resistance against natural shrink sources. If you have 'Strong Spurt' perk, maximal scale of growth spurt can be further increased by crushing your foes under this effect");
		//TutorialMessage(message, "GrowthSpurt");
		this->AllowStacking = true;
	}

	void ExplosiveGrowth::OnUpdate() {
		Actor* caster = GetCaster();
		if (!caster) {
			return;
		}
		const float GROWTH_1_POWER = 0.00125;
		const float GROWTH_2_POWER = 0.00145;
		const float GROWTH_3_POWER = 0.00175;

		auto base_spell = GetBaseEffect();

		if (base_spell == Runtime::GetMagicEffect("explosiveGrowth1")) {
			this->power = GROWTH_1_POWER;
			if (Runtime::HasPerk(caster, "ExtraGrowthMax")) {
				this->grow_limit = 2.01; // NOLINT
				this->power *= 2.0; // NOLINT
			} else if (Runtime::HasPerk(caster, "ExtraGrowth")) {
				this->grow_limit = 1.67; // NOLINT
			} else {
				this->grow_limit = 1.34; // NOLINT
			}
		} else if (base_spell == Runtime::GetMagicEffect("explosiveGrowth2")) {
			this->power = GROWTH_2_POWER;
			if (Runtime::HasPerk(caster, "ExtraGrowthMax")) {
				this->grow_limit = 2.34; // NOLINT
				this->power *= 2.0; // NOLINT
			} else if (Runtime::HasPerk(caster, "ExtraGrowth")) {
				this->grow_limit = 2.01; // NOLINT
			} else {
				this->grow_limit = 1.67; // NOLINT
			}
		} else if (base_spell == Runtime::GetMagicEffect("explosiveGrowth3")) {
			this->power = GROWTH_3_POWER;
			if (Runtime::HasPerk(caster, "ExtraGrowthMax")) {
				this->grow_limit = 2.67; // NOLINT
				this->power *= 2.0; // NOLINT
			} else if (Runtime::HasPerk(caster, "ExtraGrowth")) {
				this->grow_limit = 2.34; // NOLINT
			} else {
				this->grow_limit = 2.01; // NOLINT
			}
		}
		auto& sizemanager = SizeManager::GetSingleton();
		float AdjustLimit = std::clamp(Runtime::GetFloatOr("CrushGrowthStorage", 0.0) + 1.0f, 1.0f, 12.0f);
		float Gigantism = 1.0 + sizemanager.GetEnchantmentBonus(caster)/100;
		float scale = get_visual_scale(caster);

		float bonus = 1.0;
		float limit = this->grow_limit * Gigantism * AdjustLimit;
		float MaxSize = get_max_scale(caster) - 0.004;

		float HpRegen = GetMaxAV(caster, ActorValue::kHealth) * 0.00020;


		if (Runtime::HasPerk(caster, "HealthRegenPerk")) {
			caster->AsActorValueOwner()->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, ActorValue::kHealth, HpRegen * TimeScale());
		}

		if (scale < limit && scale < MaxSize) {
			if (Runtime::HasMagicEffect(PlayerCharacter::GetSingleton(), "EffectSizeAmplifyPotion")) {
				bonus = get_visual_scale(caster) * 0.25 + 0.75;
			}
			DoGrowth(caster, this->power * bonus);
			//log::info("Limit: {}, MaxSize: {}, Scale: {}", limit, MaxSize, scale);
		}
	}

	void ExplosiveGrowth::OnFinish() {
		Actor* caster = GetCaster();
		if (!caster) {
			return;
		}
		ExplosiveGrowth::DoShrink(caster);
	}

	void ExplosiveGrowth::DoGrowth(Actor* actor, float value) {
		update_target_scale(actor, value, SizeEffectType::kGrow); // Grow
		if (SizeManager::GetSingleton().BalancedMode() >= 2.0) {
			float scale = get_visual_scale(actor);
			if (scale >= 1.0) {
				value /= (1.5 + (scale/1.5));
			}
		}
		if (SizeManager::GetSingleton().GetGrowthSpurt(actor) < (this->grow_limit - get_natural_scale(actor))) {
			if (this->AllowStacking) {
				SizeManager::GetSingleton().ModGrowthSpurt(actor, value);
			}
		} else {
			this->AllowStacking = false;
		}

		GRumble::Once("ExplosiveGrowth", actor, get_visual_scale(actor) * 2, 0.05);
		if (this->timerSound.ShouldRunFrame()) {
			Runtime::PlaySound("xlRumbleL", actor, this->power/20, 0.0);
		}
		if (this->timer.ShouldRun()) {
			float Volume = clamp(0.20, 2.0, get_visual_scale(actor)/8);
			Runtime::PlaySoundAtNode("growthSound", actor, Volume, 1.0, "NPC Pelvis [Pelv]");
		}
	}

	void ExplosiveGrowth::DoShrink(Actor* actor) {
		float value = SizeManager::GetSingleton().GetGrowthSpurt(actor);
		update_target_scale(actor, -value, SizeEffectType::kNeutral); // Do Shrink
		if (get_target_scale(actor) <= get_natural_scale(actor)) {
			set_target_scale(actor, get_natural_scale(actor));
		}
		//log::info("Doing Shrink: {}", value);
		SizeManager::GetSingleton().SetGrowthSpurt(actor, 0.0);

		this->AllowStacking = true;

		GRumble::Once("ExplosiveGrowth", actor, 7.0, 0.05);
		if (this->timerSound.ShouldRunFrame()) {
			Runtime::PlaySound("xlRumbleL", actor, this->power/20, 0.0);
		}
		if (this->timer.ShouldRun()) {
			float Volume = clamp(0.12, 2.0, get_visual_scale(actor)/4);
			Runtime::PlaySound("shrinkSound", actor, Volume, 0.0);
		}
	}
}
