#include "managers/GtsSizeManager.hpp"
#include "magic/effects/absorb_effect.hpp"
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

		float shrink_power = 4.0 * size_difference;
		float gain_size = 0.025;

		TransferSize(caster, target, true, shrink_power, gain_size, false, ShrinkSource::magic);

		static Timer MoanTimer = Timer(10.0);
		auto random = rand() % 8;

		if (ShrinkToNothing(caster, target) && random < 2) { // chance to receive more size xp and grow even bigger
			if (MoanTimer.ShouldRunFrame()) {
				shake_camera_at_node(caster, "NPC COM [COM ]", 24.0, 0.20);
				SpawnProgressionParticle(target, true);
				ModSizeExperience(caster, 0.14);
				PlayMoanSound(caster, 1.0);
				Grow(caster, 0, 0.10);	
			}
		}
	}
	

	void Absorb::OnFinish() {
		auto caster = GetCaster();
		auto target = GetTarget();
		Task_TrackSizeTask(caster, target, "Absorb");
	}
}
