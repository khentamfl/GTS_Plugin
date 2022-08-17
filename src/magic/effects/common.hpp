#pragma once
#include "util.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"
// Module that handles various magic effects


namespace Gts {
	inline float TimeScale() {
		const float BASE_FPS = 60.0; // Parameters were optimised on this fps
		return (*g_delta_time) * BASE_FPS;
	}

	inline float CalcEffeciency(Actor* caster, Actor* target) {
		const float DRAGON_PEANLTY = 0.14;
		auto& runtime = Runtime::GetSingleton();
		float progression_multiplier = runtime.ProgressionMultiplier->value;
		float efficiency = clamp(0.25, 1.25, (caster->GetLevel()/target->GetLevel())) * progression_multiplier;

		if (std::string(target->GetDisplayFullName()).find("ragon") != std::string::npos) {
			efficiency *= DRAGON_PEANLTY;
		}

		return efficiency;
	}

	inline float CalcPower(Actor* actor, float scale_factor, float bonus) {
		auto& runtime = Runtime::GetSingleton();
		float progression_multiplier = runtime.ProgressionMultiplier->value;
		// y = mx +c
		// power = scale_factor * scale + bonus
		return (get_visual_scale(actor) * scale_factor + bonus) * progression_multiplier * TimeScale();
	}

	inline void Grow(Actor* actor, float scale_factor, float bonus) {
		// amount = scale * a + b
		auto& runtime = Runtime::GetSingleton();
		float sizeLimit = runtime.sizeLimit->value;
		float Scale = get_visual_scale(actor);
		if (Scale < sizeLimit)
		{mod_target_scale(actor, CalcPower(actor, scale_factor, bonus));}
	}

	inline void ShrinkActor(Actor* actor, float scale_factor, float bonus) {
		// amount = scale * a + b
		mod_target_scale(actor, -CalcPower(actor, scale_factor, bonus));
	}

	inline bool Revert(Actor* actor, float scale_factor, float bonus) {
		// amount = scale * a + b
		float amount = CalcPower(actor, scale_factor, bonus);
		float target_scale = get_target_scale(actor);
		float natural_scale = get_natural_scale(actor);

		if (fabs(target_scale - natural_scale) < amount) {
			set_target_scale(actor, natural_scale);
			return false;
		} else if (target_scale < natural_scale) { // NOLINT
			mod_target_scale(actor, amount);
		} else {
			mod_target_scale(actor, -amount);
		}
		return true;
	}

	inline void Steal(Actor* from, Actor* to, float scale_factor, float bonus, float effeciency) {
		effeciency = clamp(0.0, 1.0, effeciency);
		float amount = CalcPower(from, scale_factor, bonus);
		auto& runtime = Runtime::GetSingleton();
		float sizeLimit = runtime.sizeLimit->value;
		float Scale = get_visual_scale(to);
		mod_target_scale(from, -amount);
		if (Scale < sizeLimit)
		{mod_target_scale(to, amount*effeciency);}
	}

	inline void Transfer(Actor* from, Actor* to, float scale_factor, float bonus) {
		Steal(from, to, scale_factor, bonus, 1.0); // 100% efficent for friendly steal
	}


	inline void TransferSize(Actor* caster, Actor* target, bool dual_casting, float power, float transfer_effeciency, bool smt) {
		const float BASE_POWER = 0.0005;
		const float DUAL_CAST_BONUS = 2.0;
		const float SMT_BONUS = 2.0;
		const float PERK1_BONUS = 1.33;
		const float PERK2_BONUS = 2.0;

		transfer_effeciency = clamp(0.0, 1.0, transfer_effeciency); // Ensure we cannot grow more than they shrink
		
		float target_scale = get_visual_scale(target);
		float caster_scale = get_visual_scale(caster);

		power *= BASE_POWER * CalcEffeciency(caster, target);

		if (dual_casting) {
			power *= DUAL_CAST_BONUS;
		}

		if (smt) {
			power *= SMT_BONUS;
		}

		if (caster->HasPerk(runtime.PerkPart1)) {
			power *= PERK1_BONUS;
		}
		if (caster->HasPerk(runtime.PerkPart2)) {
			power *= PERK2_BONUS;
		}
		if (Runtime::GetSingleton().ProtectEssentials->value == 1.0 && target->IsEssential()) {
			return;
		}

		float alteration_level_bonus = caster->GetActorValue(ActorValue::kAlteration) * 0.00166 / 50;
		Steal(caster, target, power, power*alteration_level_bonus, transfer_effeciency);
	}

	inline bool ShrinkToNothing(Actor* caster, Actor* target) {
		const float SHRINK_TO_NOTHING_SCALE = 0.10;
		float target_scale = get_visual_scale(target);
		auto& runtime = Runtime::GetSingleton();
		if (target_scale <= SHRINK_TO_NOTHING_SCALE && target->HasMagicEffect(runtime.ShrinkToNothing) == false && target->IsPlayerTeammate() == false) {
			caster->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant)->CastSpellImmediate(runtime.ShrinkToNothingSpell, false, target, 1.00f, false, 0.0f, caster);
			return true; // NOLINT
		}
		return false;
	}
}
