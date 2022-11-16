#include "managers/GrowthTremorManager.hpp"
#include "managers/GtsManager.hpp"
#include "magic/effects/shrink_back_other.hpp"
#include "magic/effects/common.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"
#include "util.hpp"
#include "timer.hpp"

namespace Gts {
	std::string ShrinkBackOther::GetName() {
		return "ShrinkBackOther";
	}

	bool ShrinkBackOther::StartEffect(EffectSetting* effect) { // NOLINT
		auto& runtime = Runtime::GetSingleton();
		return effect == runtime.GetMagicEffect("ShrinkBackNPC");
	}

	void ShrinkBackOther::OnStart() {
		Actor* target = GetTarget();
		if (!target) {
			return;
		}
		auto& runtime = Runtime::GetSingleton();
		float Volume = clamp(0.50, 1.0, get_target_scale(target));
		runtime.PlaySound("shrinkSound", target, Volume, 0.0);
	}

	void ShrinkBackOther::OnUpdate() {
		auto target = GetTarget();
		auto caster = GetCaster();
		if (!target || !caster) {
			return;
		}
		float Power = 0.00065;
		auto& runtime = Runtime::GetSingleton();

		if (DualCasted()) {
			Power *= 2.0;
		}

		if (this->timer.ShouldRun()) {
			float Volume = clamp(0.15, 2.0, get_target_scale(target)/4);
			runtime.PlaySound("shrinkSound", target, Volume, 0.0);
			GrowthTremorManager::GetSingleton().CallRumble(target, caster, 0.60);
		}

		if (!Revert(target, Power, Power/2.5)) {
			// Returns false when shrink back is complete
			Dispel();
		}
	}
}
