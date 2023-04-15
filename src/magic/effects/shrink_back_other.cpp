
#include "managers/GtsManager.hpp"
#include "magic/effects/shrink_back_other.hpp"
#include "magic/effects/common.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"
#include "timer.hpp"
#include "managers/Rumble.hpp"

namespace Gts {
	std::string ShrinkBackOther::GetName() {
		return "ShrinkBackOther";
	}

	void ShrinkBackOther::OnStart() {
		Actor* target = GetTarget();
		if (!target) {
			return;
		}
		float Volume = clamp(0.50, 1.0, get_target_scale(target));
		Runtime::PlaySound("shrinkSound", target, Volume, 0.0);
	}

	void ShrinkBackOther::OnUpdate() {
		auto target = GetTarget();
		auto caster = GetCaster();
		if (!target || !caster) {
			return;
		}
		float Power = 0.00065;

		if (DualCasted()) {
			Power *= 2.0;
		}

		if (this->timer.ShouldRun()) {
			float Volume = clamp(0.15, 2.0, get_target_scale(target)/4);
			Runtime::PlaySound("shrinkSound", target, Volume, 0.0);
			Rumble::Once("ShrinkBackOther", target, 0.6, 0.05);
		}

		if (!Revert(target, Power, Power/2.5)) {
			// Returns false when shrink back is complete
			Dispel();
		}
	}
}
