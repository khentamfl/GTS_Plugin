#include "managers/GtsSizeManager.hpp"
#include "magic/effects/absorb_effect.hpp"
#include "magic/effects/common.hpp"
#include "utils/actorUtils.hpp"
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

	void Absorb::OnStart() {
		auto target = GetTarget();
		if (!target) {
			return;
		}
		StaggerActor(target);
	}

	void Absorb::OnUpdate() {
		const float SMT_BONUS = 1.0;
		const float TRUE_ABSORB_UPGRADE = 4.0;

		auto caster = GetCaster();
		if (!caster) {
			return;
		}
		auto target = GetTarget();
		if (!target) {
			return;
		}

		float size_difference = GetSizeDifference(caster, target);
		float gigantism = 1.0 + SizeManager::GetSingleton().GetEnchantmentBonus(caster)/100;
		if (IsEssential(target)) {
			return; // Disallow shrinking Essentials
		}
		if (HasSMT(caster)) {
			size_difference += SMT_BONUS;
		} // Insta-absorb if SMT is active

		if (size_difference >= TRUE_ABSORB_UPGRADE && !this->true_absorb) {
			// Upgrade to true absorb
			this->true_absorb = true;
		}
		if (size_difference >= 4.0) {
			size_difference = 4.0;
		} // Cap Size Difference

		if (this->true_absorb) {
			AbsorbSteal(target, caster, (0.00440 * size_difference) * gigantism, 0.0, 0.276);
			if (ShrinkToNothing(caster, target)) {
				//Dispel(); <- maybe no need to dispel since it will allow to absorb again?
			}
		} else {
			AbsorbSteal(target, caster, (0.0040 * size_difference) * gigantism, 0.0, 0.2);
		}
	}

	void Absorb::OnFinish() {
		auto caster = GetCaster();
		auto target = GetTarget();
		CastTrackSize(caster, target);
	}
}
