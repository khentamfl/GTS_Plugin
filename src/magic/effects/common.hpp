#pragma once
#include "util.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"
// Module that handles various magic effects


namespace Gts {
	inline float time_scale() {
		return *g_delta_time; // * 60.0
	}

	inline float calc_effeciency(Actor* caster, Actor* target) {
		auto& runtime = Runtime::GetSingleton();
		float ProgressionMultiplier = runtime.ProgressionMultiplier->value;
		float Efficiency = clamp(0.25, 1.25, (caster->GetLevel()/target->GetLevel())) * ProgressionMultiplier;

		if (std::string(target->GetDisplayFullName()).find("ragon") != std::string::npos) {
			Efficiency *= 0.14;
		}

		return Efficiency;
	}

	inline float CalcPower(Actor* actor, float scale_factor, float bonus) {
		auto& runtime = Runtime::GetSingleton();
		float progression_multiplier = runtime.ProgressionMultiplier->value;
		// y = mx +c
		// power = scale_factor * scale + bonus
		return (get_visual_scale(actor) * scale_factor + bonus) * progression_multiplier * time_scale();
	}

	inline void Grow(Actor* actor, float a, float b) {
		// amount = scale * a + b
		mod_target_scale(actor, CalcPower(actor, a, b));
	}

	inline void Shrink(Actor* actor, float a, float b) {
		// amount = scale * a + b
		mod_target_scale(actor, -CalcPower(actor, a, b));
	}

	inline void Revert(Actor* actor, float a, float b) {
		// amount = scale * a + b
		float amount = CalcPower(actor, a, b);
		float scale = get_visual_scale(actor);
		float natural_scale = get_natural_scale(actor);

		if (fabs(scale - natural_scale) < amount) {
			set_target_scale(actor, natural_scale);
			//Dispel(); <- Can't get it to work
		} 
		else if (scale < natural_scale) { 
			mod_target_scale(actor, amount); 
		} 
		else {
			mod_target_scale(actor, -amount);
		}
	}

	inline void Steal(Actor* from, Actor* to, float mod, float effeciency) {
		effeciency = clamp(0.0, 1.0, effeciency);
		float amount = CalcPower(from, mod, 0);
		mod_target_scale(from, -amount);
		mod_target_scale(to, amount*effeciency);
	}

	inline void Transfer(Actor* from, Actor* to, float mod, float amt) {
		Steal(from, to, mod, 1.0); // 100% efficent for friendly steal
	}


	inline void transfer_size(Actor* caster, Actor* target, bool dual_casting, float power, float transfer_effeciency, bool smallMassiveThreat) {
		transfer_effeciency = clamp(0.0, 1.0, transfer_effeciency); // Ensure we cannot grow more than they shrink
		auto& runtime = Runtime::GetSingleton();

		float TargetScale = get_visual_scale(target);
		float casterScale = get_visual_scale(caster);

		float AdditionalShrinkValue = 1.0;
		float SMTRate = 1.0;
		float Efficiency = calc_effeciency(caster, target);

		float DualCast = 1.0;
		if (dual_casting) {
			DualCast = 2.0;
		}

		if (smallMassiveThreat) {
			SMTRate = 2.0;
		}

		if (caster->HasPerk(runtime.PerkPart1)) {
			AdditionalShrinkValue = 1.33;
		} else if (caster->HasPerk(runtime.PerkPart2)) {
			AdditionalShrinkValue = 2.0;
		}
		if (Runtime::GetSingleton().ProtectEssentials->value == 1.0 && target->IsEssential() == true) {
			return;
		}
		float AlterationLevel = (caster->GetActorValue(ActorValue::kAlteration) * 0.00166 / 50) * AdditionalShrinkValue * DualCast;
		//log::info("Caster is {}", caster->GetDisplayFullName());
		//log::info("Target is {}", target->GetDisplayFullName());
		float stolen_amount = (TargetScale * 0.0005 + AlterationLevel * SMTRate * Efficiency) * power;
		mod_target_scale(target, -stolen_amount * time_scale());
		float growth_amount = stolen_amount/3 * transfer_effeciency;
		mod_target_scale(caster, growth_amount * time_scale());

		if (TargetScale <= 0.10 && target->HasMagicEffect(runtime.ShrinkToNothing) == false && target->IsPlayerTeammate() == false) {
			caster->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant)->CastSpellImmediate(runtime.ShrinkToNothingSpell, false, target, 1.00f, false, 0.0f, caster);
		}
	}
}