#include "managers/GrowthTremorManager.hpp"
#include "magic/effects/shrink_back.hpp"
#include "magic/effects/common.hpp"
#include "managers/GtsManager.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"
#include "util.hpp"
#include "timer.hpp"

namespace Gts {
	std::string ShrinkBack::GetName() {
		return "ShrinkBack";
	}

	bool ShrinkBack::StartEffect(EffectSetting* effect) { // NOLINT
		auto& runtime = Runtime::GetSingleton();
		return effect == runtime.GetMagicEffect("ShrinkBack");
	}

	void ShrinkBack::OnStart() {
		Actor* caster = GetCaster();
		if (!caster) {
			return;
		}
		auto& runtime = Runtime::GetSingleton();
		float Volume = clamp(0.50, 1.0, get_target_scale(caster));
		runtime.PlaySound("shrinkSound", caster, Volume, 1.0);
	}

	void ShrinkBack::OnUpdate() {
		auto caster = GetCaster();
		if (!caster) {
			return;
		}
		float Power = 0.00065;
		auto& runtime = Runtime::GetSingleton();

		if (DualCasted()) {
			Power *= 2.0;
		}

		if (this->timer.ShouldRun()) {
			float Volume = clamp(0.15, 2.0, get_target_scale(caster)/4);
			runtime.PlaySound("shrinkSound", caster, Volume, 0.0);
			GrowthTremorManager::GetSingleton().CallRumble(caster, caster, 0.60);
		}

		if (!Revert(caster, Power, Power/2.5)) {
			// Returns false when shrink back is complete
			Dispel();
		}
	}
}
