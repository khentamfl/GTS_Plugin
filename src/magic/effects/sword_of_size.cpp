#include "managers/animation/Utils/AnimationUtils.hpp"
#include "magic/effects/sword_of_size.hpp"
#include "magic/effects/common.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"

namespace Gts {
	std::string SwordOfSize::GetName() {
		return "SwordOfSize";
	}

	SwordOfSize::SwordOfSize(ActiveEffect* effect) : Magic(effect) {
	}

	void SwordOfSize::OnUpdate() {
		auto caster = GetCaster();
		if (!caster) {
			return;
		}
		auto target = GetTarget();
		if (!target) {
			return;
		}
		float gain = 0.02;
		float shrink = 3.0;
		if (target->IsDead())  {
			shrink *= 2.5;
			gain *= 0.20;
		}

		TransferSize(caster, target, false, 3.00, 0.02, false, ShrinkSource::magic);
	}

	void SwordOfSize::OnFinish() {
		auto Caster = GetCaster();
		auto Target = GetTarget();
		Task_TrackSizeTask(Caster, Target, "Sword");
	}
}

