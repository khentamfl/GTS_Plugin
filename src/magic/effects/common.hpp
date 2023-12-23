#pragma once
#include "managers/ShrinkToNothingManager.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/ai/aifunctions.hpp"
#include "utils/actorUtils.hpp"
#include "data/persistent.hpp"
#include "data/runtime.hpp"
#include "data/time.hpp"
#include "scale/scale.hpp"
#include "events.hpp"
// Module that handles various magic effects

namespace {
	const float MASTER_POWER = 2.0;
}

namespace Gts {
	inline float TimeScale() {
		const float BASE_FPS = 60.0; // Parameters were optimised on this fps
		return Time::WorldTimeDelta() * BASE_FPS;
	}

	inline void AdjustSizeReserve(Actor* giant, float value) {
		if (!Runtime::HasPerk(giant, "SizeReserve")) {
			return;
		}
		auto Cache = Persistent::GetSingleton().GetData(giant);
		if (Cache) {
			Cache->SizeReserve += value;
		}
	}

	inline float GetStealEfficiency(Actor* tiny) {
		float eff = 1.0;
		if (IsDragon(tiny)) {
			eff *= 0.20;
		} else if (IsMammoth(tiny)) {
			eff *= 0.35;
		} else if (IsGiant(tiny)) {
			eff *= 0.50;
		} else if (IsUndead(tiny)) {
			eff *= 0.60;
		}
		return eff;
	}

	inline void ModSizeExperience(float value, Actor* Caster) { // Adjust Matter Of Size skill
		if (Caster->formID != 0x14) {
			return; //Bye
		}
		auto GtsSkillLevel = Runtime::GetGlobal("GtsSkillLevel");
		auto GtsSkillRatio = Runtime::GetGlobal("GtsSkillRatio");
		auto GtsSkillProgress = Runtime::GetGlobal("GtsSkillProgress");


		int random = (100 + (rand()% 25 + 1)) / 100;

		if (GtsSkillLevel->value >= 100.0) {
			GtsSkillLevel->value = 100.0;
			GtsSkillRatio->value = 0.0;
			return;
		}

		float skill_level = GtsSkillLevel->value;

		float ValueEffectiveness = std::clamp(1.0 - GtsSkillLevel->value/100, 0.10, 1.0);

		float oldvaluecalc = 1.0 - GtsSkillRatio->value; //Attempt to keep progress on the next level
		float Total = (value * random) * ValueEffectiveness;
		GtsSkillRatio->value += Total * GetXpBonus();

		if (GtsSkillRatio->value >= 1.0) {
			float transfer = clamp(0.0, 1.0, Total - oldvaluecalc);
			GtsSkillRatio->value = transfer;
			GtsSkillLevel->value = skill_level + 1.0;
			GtsSkillProgress->value = GtsSkillLevel->value;
			AddPerkPoints(GtsSkillLevel->value);
		}
	}

	inline void AdjustSizeLimit(float value, Actor* caster) {  // A function that adjusts Size Limit (Globals)
		if (caster->formID != 0x14) {
			return;
		}
		float progressionMultiplier = Persistent::GetSingleton().progression_multiplier;

		auto globalMaxSizeCalc = Runtime::GetFloat("GlobalMaxSizeCalc");
		if (globalMaxSizeCalc < 10.0) {
			Runtime::SetFloat("GlobalMaxSizeCalc", globalMaxSizeCalc + (value * 1.45 * 50 * progressionMultiplier * TimeScale())); // Always apply it
		}
	}

	inline void AdjustMassLimit(float value, Actor* caster) { // Adjust Size Limit for Mass Based Size Mode
		if (caster->formID != 0x14) {
			return;
		}
		auto selectedFormula = Runtime::GetInt("SelectedSizeFormula");
		float progressionMultiplier = Persistent::GetSingleton().progression_multiplier;
		if (selectedFormula) {
			if (selectedFormula >= 1.0) {
				SoftPotential mod {
					.k = 0.070,
					.n = 3,
					.s = 0.54,
				};
				auto globalMassSize = Runtime::GetFloat("GtsMassBasedSize");
				float modifier = soft_core(globalMassSize, mod);
				if (modifier <= 0.10) {
					modifier = 0.10;
				}
				value *= 10.0 * modifier;
				//log::info("Modifier: {}", modifier);
				auto sizeLimit = Runtime::GetFloat("sizeLimit");
				if (Runtime::HasPerk(caster, "TotalControl")) {
					sizeLimit = 999999.0;
				}
				if (globalMassSize + 1.0 < sizeLimit) {
					Runtime::SetFloat("GtsMassBasedSize", globalMassSize + value * progressionMultiplier * TimeScale());
				}
			}
		}
	}

	inline float CalcEffeciency(Actor* caster, Actor* target, bool progression) {
		float progression_multiplier = 1.0;
		if (progression) {
			progression_multiplier = Persistent::GetSingleton().progression_multiplier;
		}
		float casterlevel = clamp(1.0, 500.0, caster->GetLevel());
		float targetlevel = clamp(1.0, 500.0, target->GetLevel());
		float GigantismCaster = 1.0 + SizeManager::GetSingleton().GetEnchantmentBonus(caster)*0.01;
		float SizeHunger = 1.0 + SizeManager::GetSingleton().GetSizeHungerBonus(caster)*0.01;
		float GigantismTarget = 1.0 + SizeManager::GetSingleton().GetEnchantmentBonus(target)*0.01;  // May go negative needs fixing with a smooth clamp
		float efficiency = clamp(0.50, 1.0, (casterlevel/targetlevel)) * progression_multiplier;

		float Scale_Resistance = std::clamp(get_visual_scale(target), 1.0f, 9999.0f); // Calf_power makes shrink effects stronger based on scale, this fixes that.

		efficiency *= GetStealEfficiency(target);
		if (Runtime::HasMagicEffect(target, "ResistShrinkPotion")) {
			efficiency *= 0.25;
		}

		efficiency *= (GigantismCaster / GigantismTarget) * SizeHunger;
		efficiency /= Scale_Resistance;

		return efficiency;
	}

	inline float CalcPower(Actor* actor, float scale_factor, float bonus, bool mult) {
		float progression_multiplier = 1.0;
		if (mult) {
			progression_multiplier = Persistent::GetSingleton().progression_multiplier;
		}
		// y = mx +c
		// power = scale_factor * scale + bonus
		float scale = clamp(0.5, 999999.0, get_visual_scale(actor));
		return (scale * scale_factor + bonus) * progression_multiplier * MASTER_POWER * TimeScale();
	}

	inline void Grow(Actor* actor, float scale_factor, float bonus) {
		// amount = scale * a + b
		update_target_scale(actor, CalcPower(actor, scale_factor, bonus, true), SizeEffectType::Growing);
	}

	inline void CrushGrow(Actor* actor, float scale_factor, float bonus) {
		// amount = scale * a + b
		float modifier = SizeManager::GetSingleton().BalancedMode();
		scale_factor /= modifier;
		bonus /= modifier;
		update_target_scale(actor, CalcPower(actor, scale_factor, bonus, true), SizeEffectType::Growing);
		AddStolenAttributes(actor, CalcPower(actor, scale_factor, bonus, true));
	}

	inline void ShrinkActor(Actor* actor, float scale_factor, float bonus) {
		// amount = scale * a + b
		update_target_scale(actor, -CalcPower(actor, scale_factor, bonus, true), SizeEffectType::Shrinking);
	}

	inline bool Revert(Actor* actor, float scale_factor, float bonus) {
		// amount = scale * a + b
		float amount = CalcPower(actor, scale_factor, bonus, true);
		float target_scale = get_target_scale(actor);
		float natural_scale = get_neutral_scale(actor);

		if (fabs(target_scale - natural_scale) < amount) {
			set_target_scale(actor, natural_scale);
			return false;
		} else if (target_scale < natural_scale) { // NOLINT
			update_target_scale(actor, amount, SizeEffectType::Neutral);
		} else {
			update_target_scale(actor, -amount, SizeEffectType::Neutral);
		}
		return true;
	}

	inline void Steal(Actor* from, Actor* to, float scale_factor, float bonus, float effeciency, ShrinkSource source) {
		effeciency = clamp(0.01, 1.0, effeciency);
		float effeciency_noscale = clamp(0.01, 1.0, CalcEffeciency(to, from, false));
		//log::info("Efficiency is: {}", effeciency_noscale);
		float amount = CalcPower(from, scale_factor, bonus, true);
		float amountnomult = CalcPower(from, scale_factor, bonus, false);
		float target_scale = get_visual_scale(from);
		ModSizeExperience(0.52 * scale_factor * target_scale, to);
		update_target_scale(from, -amountnomult * 0.55 * effeciency_noscale, SizeEffectType::Shrinking);
		update_target_scale(to, amount*effeciency, SizeEffectType::Growing);

		if (source == ShrinkSource::hugs) {
			AdvanceQuestProgression(to, 1.0, amountnomult * 0.55 * effeciency_noscale);
		} else {
			AdvanceQuestProgression(to, 2.0, amountnomult * 0.55 * effeciency_noscale);
		}

		AddStolenAttributes(to, amount*effeciency);
	}

	inline void AbsorbSteal(Actor* from, Actor* to, float scale_factor, float bonus, float effeciency) {
		effeciency = clamp(0.0, 1.0, effeciency);
		float amount = CalcPower(from, scale_factor, bonus, true);
		float target_scale = get_visual_scale(from);
		AdjustSizeLimit(0.0012 * scale_factor * target_scale, to);
		AdjustMassLimit(0.0012 * scale_factor* target_scale, to);
		ModSizeExperience(0.52 * scale_factor * target_scale, to);
		update_target_scale(from, -amount, SizeEffectType::Shrinking);
		update_target_scale(to, amount*effeciency/10, SizeEffectType::Growing); // < 10 times weaker size steal towards caster. Absorb exclusive.
	}

	inline void Transfer(Actor* from, Actor* to, float scale_factor, float bonus) {
		Steal(from, to, scale_factor, bonus, 1.0, ShrinkSource::other); // 100% efficent for friendly steal
	}

	inline void Grow_Ally(Actor* from, Actor* to, float receiver, float caster) {
		float receive = CalcPower(from, receiver, 0, true);
		float lose = CalcPower(from, receiver, 0, true);
		float CasterScale = get_target_scale(from);
		if (CasterScale > 1.0) { // We don't want to scale the caster below this limit!
			update_target_scale(from, -lose, SizeEffectType::Shrinking);
		}
		update_target_scale(to, receive, SizeEffectType::Growing);
	}

	inline void TransferSize(Actor* caster, Actor* target, bool dual_casting, float power, float transfer_effeciency, bool smt, ShrinkSource source) {
		const float BASE_POWER = 0.0005;
		const float DUAL_CAST_BONUS = 2.0;
		const float SMT_BONUS = 1.25;
		float PERK_BONUS = 1.0;

		if (IsEssential(target)) {
			return;
		}

		transfer_effeciency = clamp(0.0, 1.0, transfer_effeciency); // Ensure we cannot grow more than they shrink

		float target_scale = get_visual_scale(target);
		float caster_scale = get_visual_scale(caster);

		power *= BASE_POWER * CalcEffeciency(caster, target, true);

		if (dual_casting) {
			power *= DUAL_CAST_BONUS;
		}

		if (smt) {
			power *= SMT_BONUS;
		}

		if (Runtime::HasPerkTeam(caster, "FastShrink")) {
			PERK_BONUS += 0.15;
		}
		if (Runtime::HasPerkTeam(caster, "LethalShrink")) {
			PERK_BONUS += 0.35;
		}

		power *= PERK_BONUS; // multiply power by perk bonuses

		AdjustSizeLimit(0.0300 * target_scale * power, caster);
		AdjustMassLimit(0.0160 * target_scale * power, caster);

		auto GtsSkillLevel = GetGtsSkillLevel();

		float alteration_level_bonus = 0.0380 + (GtsSkillLevel * 0.000360); // + 100% bonus at level 100
		Steal(target, caster, power, power * alteration_level_bonus, transfer_effeciency, source);
	}

	inline bool ShrinkToNothing(Actor* caster, Actor* target) {
		float SHRINK_TO_NOTHING_SCALE = 0.08;
		float target_scale = get_visual_scale(target);
		if (!caster) {
			return false;
		}
		if (IsDragon(target) || IsMammoth(target)) {
			SHRINK_TO_NOTHING_SCALE = 0.024;
		}
		if (IsGiant(target)) {
			SHRINK_TO_NOTHING_SCALE = 0.044;
		}

		if (target_scale <= SHRINK_TO_NOTHING_SCALE && !Runtime::HasMagicEffect(target,"ShrinkToNothing") && !target->IsPlayerTeammate()) {
			if (!ShrinkToNothingManager::CanShrink(caster, target)) {
				return false;
			}

			if (!target->IsDead()) {
				if (IsGiant(target)) {
					AdvanceQuestProgression(caster, target, 7, 1, false);
				} else {
					AdvanceQuestProgression(caster, target, 4, 1, false);
				}
			}

			AdjustSizeLimit(0.0060, caster);
			AdjustMassLimit(0.0060, caster);

			auto Cache = Persistent::GetSingleton().GetData(caster);

			if (!Cache) {
				return false;
			}

			AdjustSizeReserve(caster, target_scale/25);
			PrintDeathSource(caster, target, DamageSource::Shrinked);
			ShrinkToNothingManager::Shrink(caster, target);
			return true;
		}
		return false;
	}

	inline void CrushBonuses(Actor* caster, Actor* target) {
		float target_scale = get_target_scale(target);
		float caster_scale = get_target_scale(caster);
		if (IsDragon(target)) {
			target_scale *= 2.0;
		}
		auto player = PlayerCharacter::GetSingleton();
		float sizedifference = caster_scale/target_scale;
		float instacrushrequirement = 24.0;

		if (IsEssential(target)) {
			return;
		}
		int Random = rand() % 8;
		if (Random >= 8 && Runtime::HasPerk(caster, "GrowthDesirePerk")) {
			Runtime::PlaySoundAtNode("MoanSound", caster, 1.0, 1.0, "NPC Head [Head]");
		}

		bool GTSBusy;
		caster->GetGraphVariableBool("GTS_Busy", GTSBusy);

		auto Cache = Persistent::GetSingleton().GetData(caster); // TODO: Fix this properly
		if (!Cache) {
			return;
		}
		if (caster == player) {
			AdjustSizeReserve(caster, target_scale/25);
			AdjustSizeLimit(0.0066 * target_scale, caster);
			AdjustMassLimit(0.0066 * target_scale, caster);
			if (Runtime::HasPerk(caster, "ExtraGrowth") && HasGrowthSpurt(caster)) {
				auto CrushGrowthStorage = Runtime::GetFloat("CrushGrowthStorage");
				Runtime::SetFloat("CrushGrowthStorage", CrushGrowthStorage + (target_scale/75) / SizeManager::GetSingleton().BalancedMode());
			}
			// Slowly increase Crush Growth Limit after crushing someone while Growth Spurt is active.
		}
	}


	inline void CastTrackSize(Actor* caster, Actor* target) {
		Runtime::CastSpell(caster, target, "TrackSizeSpell");
	}
}
