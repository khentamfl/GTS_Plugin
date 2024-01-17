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
		StaggerActor(target, 100.0f);
	}

	void Absorb::OnUpdate() {
		const float SMT_BONUS = 1.0;

		auto caster = GetCaster();
		if (!caster) {
			return;
		}
		auto target = GetTarget();
		if (!target) {
			return;
		}

		if (target == caster) {
			return;
		}

		float size_difference = GetSizeDifference(caster, target);
		float gigantism = 1.0 + Ench_Aspect_GetPower(caster);
		if (IsEssential(target)) {
			return; // Disallow shrinking Essentials
		}
		if (HasSMT(caster)) {
			size_difference += SMT_BONUS;
		} // More shrink with SMT

		
		if (size_difference >= 3.0) {
			size_difference = 3.0;
		} // Cap Size Difference

		
		AbsorbSteal(target, caster, (0.0008 * size_difference) * gigantism, 0.0, 0.20);
		if (ShrinkToNothing(caster, target)) {
			//
		}
	}
	

	void Absorb::OnFinish() {
		auto caster = GetCaster();
		auto target = GetTarget();
		CastTrackSize(caster, target);
	}
}
