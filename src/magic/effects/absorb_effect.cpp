#include "magic/effects/absorb_effect.hpp"
#include "magic/effects/common.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"

namespace Gts {
	Absorb::Absorb(ActiveEffect* effect) : Magic(effect) {
		auto base_spell = GetBaseEffect();
		auto& runtime = Runtime::GetSingleton();

		if (base_spell == runtime.TrueAbsorb) {
			this->true_absorb = true;
		} else {
			this->true_absorb = false;
		}
	}

	bool Absorb::StartEffect(EffectSetting* effect) {
		auto& runtime = Runtime::GetSingleton();
		return (effect == runtime.AbsorbMGEF || effect == runtime.TrueAbsorb);
	}

	void Absorb::OnUpdate() {
		auto caster = GetCaster();
		if (!caster) {
			return;
		}
		auto target = GetTarget();
		if (!target) {
			return;
		}

		auto& runtime = Runtime::GetSingleton();
		float ProgressionMultiplier = runtime.ProgressionMultiplier->value;
		float casterScale = get_visual_scale(caster);
		float targetScale = get_visual_scale(target);
		float SizeDifference = casterScale/targetScale;
		if (this->true_absorb) {
			mod_target_scale(target, -0.00825 * ProgressionMultiplier * SizeDifference);
			mod_target_scale(caster,  0.00250 * ProgressionMultiplier * targetScale);
			if (targetScale <= 0.25 && target->HasMagicEffect(runtime.ShrinkToNothing) == false) {
				caster->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant)->CastSpellImmediate(runtime.ShrinkToNothingSpell, false, target, 1.00f, false, 0.0f, caster);
			}
			// ^ Emulate absorption
		} else {
			mod_target_scale(target, -0.0025 * ProgressionMultiplier * SizeDifference);
			mod_target_scale(caster,  0.0005 * ProgressionMultiplier * targetScale);
		}

	}
}
