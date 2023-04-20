#include "managers/GtsSizeManager.hpp"
#include "magic/effects/absorb_effect.hpp"
#include "magic/effects/common.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"

namespace Gts {
	Absorb::Absorb(ActiveEffect* effect) : Magic(effect) {
		auto base_spell = GetBaseEffect();

		this->true_absorb = (base_spell == Runtime::GetMagicEffect("TrueAbsorb"));
	}

	std::string Absorb::GetName() {
		return "Absorb";
	}

	void Absorb::OnUpdate() {
		const float SMT_BONUS = 3.0;
		const float TRUE_ABSORB_UPGRADE = 4.0;

		auto caster = GetCaster();
		if (!caster) {
			return;
		}
		auto target = GetTarget();
		if (!target) {
			return;
		}

		float caster_scale = get_visual_scale(caster);
		float target_scale = get_visual_scale(target);
		float size_difference = caster_scale/target_scale;
		float gigantism = 1.0 + SizeManager::GetSingleton().GetEnchantmentBonus(caster)/100;
		if (Runtime::HasMagicEffect(caster, "SmallMassiveThreat")) {
			size_difference += SMT_BONUS;
		} // Insta-absorb if SMT is active

		if (size_difference >= TRUE_ABSORB_UPGRADE && !this->true_absorb) {
			// Upgrade to true absorb
			this->true_absorb = true;
		}
		if (size_difference >= 4.1) {
			size_difference = 4.1;
		} // Cap Size Difference

		if (this->true_absorb) {
			AbsorbSteal(target, caster, (0.00070 * size_difference) * gigantism, 0.0, 0.276);
			if (ShrinkToNothing(caster, target)) {
				//Dispel(); <- maybe no need to dispel since it will allow to absorb again?
			}
		} else {
			AbsorbSteal(target, caster, (0.0010 * size_difference) * gigantism, 0.0, 0.2);
		}
	}

	void Absorb::OnFinish() {
		auto caster = GetCaster();
		auto target = GetTarget();
		CastTrackSize(caster, target);
	}
}
