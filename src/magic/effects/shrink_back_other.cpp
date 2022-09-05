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
		return effect == runtime.ShrinkBackNPC;
	}

	void ShrinkBackOther::OnStart() {
		Actor* target = GetTarget();
		if (!target) {
			return;
		}
		auto& runtime = Runtime::GetSingleton();
		auto ShrinkSound = runtime.shrinkSound;
		float Volume = clamp(0.50, 1.0, get_visual_scale(target));
		PlaySound(ShrinkSound, target, Volume, 0.0);
		log::info("Shrink Sound Start, actor: {}", caster->GetDisplayFullName());
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
			auto ShrinkSound = runtime.shrinkSound;
			float Volume = clamp(0.15, 2.0, get_visual_scale(target)/4);
			PlaySound(ShrinkSound, target, Volume, 0.0);
			GrowthTremorManager::GetSingleton().CallRumble(target, caster, 0.30);
			log::info("Shrink Sound Loop, actor: {}", caster->GetDisplayFullName());
		}

		if (!Revert(target, Power, Power/2.5)) {
			// Returns false when shrink back is complete
			Dispel();
		}
	}
}
