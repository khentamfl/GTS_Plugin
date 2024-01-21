#include "managers/GtsSizeManager.hpp"
#include "magic/effects/absorb_effect.hpp"
#include "magic/effects/common.hpp"
#include "utils/actorUtils.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"

namespace Gts {
	std::string Absorb::GetName() {
		return "Absorb";
	}

	void Absorb::OnStart() {
		auto target = GetTarget();
		if (!target) {
			return;
		} 
		auto caster = GetCaster();
		if (!caster) {
			return;
		}
		StaggerActor(target, 25.0f * GetSizeDifference(caster, target));
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
		if (IsEssential(target)) {
			return; // Disallow shrinking Essentials
		}
		if (HasSMT(caster)) {
			size_difference += SMT_BONUS;
		} // More shrink with SMT

		
		if (size_difference >= 3.0) {
			size_difference = 3.0;
		} // Cap Size Difference

		TransferSize(caster, target, true, (0.0008 * size_difference), 0.0, 0.025, false, ShrinkSource::magic);
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
