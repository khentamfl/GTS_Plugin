#pragma once
#include "util.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"
// Module that handles footsteps
#include <SKSE/SKSE.h>

namespace Gts {
	inline float calc_effeciency(Actor* caster, Actor* target) {
		auto& runtime = Runtime::GetSingleton();
		float ProgressionMultiplier = runtime.ProgressionMultiplier->value;
		float Efficiency = clamp(0.25, 1.25, (caster->GetLevel()/target->GetLevel())) * ProgressionMultiplier;

		if (std::string(target->GetDisplayFullName()).find("ragon") != std::string::npos) {
			Efficiency *= 0.86;
		}

		return Efficiency;
	}

	inline float transfer_size(Actor* caster, Actor* target, bool dual_casting, float power, float transfer_effeciency, bool smallMassiveThreat) {
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

		float AlterationLevel = (caster->GetActorValue(ActorValue::kAlteration) * 0.00166 / 50) * AdditionalShrinkValue * DualCast;

		if (TargetScale < 0.25) {
			auto source = caster->GetMagicCaster(MagicSystem::CastingSource::kLeftHand);
			if (source) {
				source->CastSpellImmediate(runtime.ShrinkToNothingSpell, false, target, 1, false, 1, caster);
			}
		}

		float stolen_amount = (TargetScale * 0.0015 + AlterationLevel * SMTRate * Efficiency) * power;
		mod_target_scale(target, -stolen_amount);
		float growth_amount = stolen_amount * transfer_effeciency;
		mod_target_scale(caster, growth_amount);

	}
}
