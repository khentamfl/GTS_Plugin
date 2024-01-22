#include "managers/animation/Utils/AnimationUtils.hpp"
#include "magic/effects/absorb_effect.hpp"
#include "managers/GtsSizeManager.hpp"
#include "magic/effects/common.hpp"
#include "utils/actorUtils.hpp"
#include "data/runtime.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"


#include "timer.hpp"

namespace Gts {
	Absorb::Absorb(ActiveEffect* effect) : Magic(effect) {}

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
		StaggerActor(target, 0.25f * GetSizeDifference(caster, target));
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
		if (IsEssential(target)) {
			return; // Disallow shrinking Essentials
		}

		float size_difference = GetSizeDifference(caster, target);

		if (HasSMT(caster)) {
			size_difference += SMT_BONUS;
		} // More shrink with SMT

		
		if (size_difference >= 2.0) {
			size_difference = 2.0;
		} // Cap Size Difference

		float shrink_power = 2.0 * size_difference;
		float gain_size = 0.0025;

		if (target->IsDead()) {
			shrink_power *= 2.5;
			gain_size *= 0.20;
		}

		TransferSize(caster, target, true, shrink_power, gain_size, false, ShrinkSource::magic);

		if (ShrinkToNothing(caster, target)) { // chance to receive more size xp and grow even bigger
			AbsorbShout_BuffCaster(caster, target);
		}
	}
	

	void Absorb::OnFinish() {
		auto caster = GetCaster();
		auto target = GetTarget();
		Task_TrackSizeTask(caster, target, "Absorb");
	}
}
