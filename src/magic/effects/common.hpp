#pragma once
#include "util.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"
#include "data/time.hpp"
// Module that handles various magic effects

namespace {
	const float MASTER_POWER = 2.0;
}

namespace Gts {
	inline float TimeScale() {
		const float BASE_FPS = 60.0; // Parameters were optimised on this fps
		return Time::WorldTimeDelta() * BASE_FPS;
	}


	inline void AdjustSizeLimit(float Value)  // A function that adjusts Size Limit (Globals)
	{
		//AdjustMaxSize = 20CAC5
		//AdjustMaxSize_MassBased = 277005
		auto& runtime = Runtime::GetSingleton();

		float GetGlobalMaxSizeCalc = runtime.GlobalMaxSizeCalc->value; // <- Bonus that is used to determine quest progression
		float GetGlobalMassSize = runtime.MassBasedSizeLimit->value; // <- Applies it
		float ProgressionMultiplier = runtime.ProgressionMultiplier->value;

		float SelectedFormula = runtime.SelectedSizeFormula->value;
		float SizeLimit = runtime.sizeLimit->value;

		if (GetGlobalMaxSizeCalc < 10.0) {
			runtime.GlobalMaxSizeCalc->value = GetGlobalMaxSizeCalc + (Value * 10 * ProgressionMultiplier * TimeScale()); // Always apply it
		}

		if (GetGlobalMassSize < SizeLimit && SelectedFormula >= 2.0) {
			runtime.MassBasedSizeLimit->value = GetGlobalMassSize + (Value  * ProgressionMultiplier * TimeScale());
		}
	}


	inline float CalcEffeciency(Actor* caster, Actor* target) {
		const float DRAGON_PEANLTY = 0.14;
		auto& runtime = Runtime::GetSingleton();
		float progression_multiplier = runtime.ProgressionMultiplier->value;
		float GigantismCaster = 1.0 + Persistent::GetSingleton().GetActorData(caster)->gigantism_enchantment;
		float GigantismTarget = 1.0 - Persistent::GetSingleton().GetActorData(target)->gigantism_enchantment;
		float efficiency = clamp(0.25, 1.25, (caster->GetLevel()/target->GetLevel())) * progression_multiplier;
		if (std::string(target->GetDisplayFullName()).find("ragon") != std::string::npos) {
			efficiency *= DRAGON_PEANLTY;
		}
		if (target->HasMagicEffect(runtime.ResistShrinkPotion)) {
			efficiency *= 0.25;
		}

		effeciency *= GigantismCaster * GigantismTarget;

		return efficiency;
	}

	inline float CalcPower(Actor* actor, float scale_factor, float bonus) {
		auto& runtime = Runtime::GetSingleton();
		float progression_multiplier = runtime.ProgressionMultiplier->value;
		// y = mx +c
		// power = scale_factor * scale + bonus
		return (get_visual_scale(actor) * scale_factor + bonus) * progression_multiplier * MASTER_POWER * TimeScale();
	}

	inline void Grow(Actor* actor, float scale_factor, float bonus) {
		// amount = scale * a + b
		mod_target_scale(actor, CalcPower(actor, scale_factor, bonus));
	}

	inline void CrushGrow(Actor* actor, float scale_factor, float bonus) {
		// amount = scale * a + b
		mod_target_scale(actor, CalcPower(actor, scale_factor, bonus));
		log::info("Crush Growth Active");
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
		float target_scale = get_visual_scale(from);
		AdjustSizeLimit(0.0001 * target_scale);
		mod_target_scale(from, -amount);
		mod_target_scale(to, amount*effeciency);
	}

	inline void AbsorbSteal(Actor* from, Actor* to, float scale_factor, float bonus, float effeciency) {
		effeciency = clamp(0.0, 1.0, effeciency);
		float amount = CalcPower(from, scale_factor, bonus);
		float target_scale = get_visual_scale(from);
		AdjustSizeLimit(0.0016 * target_scale);
		mod_target_scale(from, -amount);
		mod_target_scale(to, amount*effeciency/10); // < 4 times weaker size steal towards caster. Absorb exclusive.
	}

	inline void Transfer(Actor* from, Actor* to, float scale_factor, float bonus) {
		Steal(from, to, scale_factor, bonus, 1.0); // 100% efficent for friendly steal
	}

	inline void Grow_Ally(Actor* from, Actor* to, float receiver, float caster) {
		float receive = CalcPower(from, receiver, 0);
		float lose = CalcPower(from, receiver, 0);
		float CasterScale = get_visual_scale(from);
		if (CasterScale > 1.0) { // We don't want to scale the caster below this limit!
			mod_target_scale(from, -lose);
		}
		mod_target_scale(to, receive);
	}

	inline void TransferSize(Actor* caster, Actor* target, bool dual_casting, float power, float transfer_effeciency, bool smt) {
		const float BASE_POWER = 0.0005;
		const float DUAL_CAST_BONUS = 2.0;
		const float SMT_BONUS = 2.0;
		const float PERK1_BONUS = 1.33;
		const float PERK2_BONUS = 2.0;

		auto& runtime = Runtime::GetSingleton();
		if (runtime.ProtectEssentials->value == 1.0 && target->IsEssential()) {
			return;
		}

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
		AdjustSizeLimit(0.0001 * target_scale * power);
		float alteration_level_bonus = caster->GetActorValue(ActorValue::kAlteration) * 0.00166 / 50;
		Steal(target, caster, power, power*alteration_level_bonus, transfer_effeciency);
	}

	inline bool ShrinkToNothing(Actor* caster, Actor* target) {
		const float SHRINK_TO_NOTHING_SCALE = 0.12;
		float target_scale = get_visual_scale(target);
		auto& runtime = Runtime::GetSingleton();
		if (target_scale <= SHRINK_TO_NOTHING_SCALE && !target->HasMagicEffect(runtime.ShrinkToNothing) && !target->IsPlayerTeammate()) {
			caster->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant)->CastSpellImmediate(runtime.ShrinkToNothingSpell, false, target, 1.00f, false, 0.0f, caster);
			PlaySound(runtime.BloodGushSound, target, 1.0, 1.0);
			AdjustSizeLimit(0.0117);
			ConsoleLog::GetSingleton()->Print("%s Was absorbed by %s", target->GetDisplayFullName(), caster->GetDisplayFullName());
			return true;
		}
		return false;
	}

	inline void CrushToNothing(Actor* caster, Actor* target) {
		float target_scale = get_visual_scale(target);
		auto& runtime = Runtime::GetSingleton();
		if (!caster->HasPerk(runtime.GrowthPerk)) { // Requires Growth Perk in order to grow
			return;
		}
		int Random = rand() % 8;
		if (Random >= 8) {
			PlaySound(runtime.MoanSound,caster, 1.0, 1.0);
		}
		caster->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant)->CastSpellImmediate(runtime.CrushGrowthSpell, false, target, 1.00f, false, 0.0f, caster);
		if (get_visual_scale(caster) <= 13.0 || !caster->IsSprinting() && !caster->HasMagicEffect(runtime.SmallMassiveThreat)) {
			PlaySound(runtime.BloodGushSound, target, 1.0, 1.0);
			caster->NotifyAnimationGraph("JumpLand");
			
		}
		AdjustSizeLimit(0.0417 * target_scale);
		


		if (caster->HasPerk(runtime.ExtraGrowth) && (caster->HasMagicEffect(runtime.explosiveGrowth1) || caster->HasMagicEffect(runtime.explosiveGrowth2) || caster->HasMagicEffect(runtime.explosiveGrowth3))) {
			runtime.CrushGrowthStorage->value += target_scale/50;
		} // Slowly increase Limit after crushing someone while Growth Spurt is active.


		ConsoleLog::GetSingleton()->Print("%s Was crushed by %s", target->GetDisplayFullName(), caster->GetDisplayFullName());
	}

	inline void CastTrackSize(Actor* caster, Actor* target) {
		auto& runtime = Runtime::GetSingleton();
		caster->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant)->CastSpellImmediate(runtime.TrackSizeSpell, false, target, 1.00f, false, 0.0f, caster);
	}
}
