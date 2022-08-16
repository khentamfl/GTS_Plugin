#include "magic/effects/absorb_effect.hpp"
#include "magic/effects/common.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"

namespace Gts {
	Absorb::Absorb(ActiveEffect* effect) : Magic(effect) {
		auto base_spell = GetBaseEffect();
		auto& runtime = Runtime::GetSingleton();

		this->true_absorb = (base_spell == runtime.TrueAbsorb);
	}

	bool Absorb::StartEffect(EffectSetting* effect) { // NOLINT
		auto& runtime = Runtime::GetSingleton();
		return (effect == runtime.AbsorbMGEF || effect == runtime.TrueAbsorb);
	}

	void Absorb::OnUpdate() {
		const float SMT_BONUS = 4.0;
		const float TRUE_ABSORB_UPGRADE = 4.0;
		const float SHRINK_TO_NOTHING_SCALE = 0.25;

		auto caster = GetCaster();
		if (!caster) {
			return;
		}
		auto target = GetTarget();
		if (!target) {
			return;
		}

		auto& runtime = Runtime::GetSingleton();
		float caster_scale = get_visual_scale(caster);
		float target_scale = get_visual_scale(target);
		float size_difference = caster_scale/target_scale;
		if (caster->HasMagicEffect(runtime.smallMassiveThreat)) {
			size_difference *= SMT_BONUS;
		} // Insta-absorb if SMT is active

		if (size_difference >= TRUE_ABSORB_UPGRADE && !this->true_absorb) {
			// Upgrade to true absorb
			this->true_absorb = true;
		}
		if (size_difference >= 4.0 && target->HasMagicEffect(runtime.TrueAbsorb) == false)
		{caster->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant)->CastSpellImmediate(runtime.TrueAbsorbSpell, false, target, 1.00f, false, 0.0f, caster);}
		if (this->true_absorb) {
			Steal(target, caster, 0.00325 * size_difference, 0.276);
			if (target_scale <= SHRINK_TO_NOTHING_SCALE && target->HasMagicEffect(runtime.ShrinkToNothing) == false) {
				// Shrink to nothing
				caster->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant)->CastSpellImmediate(runtime.ShrinkToNothingSpell, false, target, 1.00f, false, 0.0f, caster);
				//Dispel();
				return;
			}
		} else {
			Steal(target, caster, 0.0025 * size_difference, 0.2);
		}
	}
}
