#include "managers/GrowthTremorManager.hpp"
#include "managers/GtsManager.hpp"
#include "magic/effects/shrink_back_other.hpp"
#include "magic/effects/common.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"
#include "util.hpp"

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
	}

	void ShrinkBackOther::OnUpdate() {
		auto target = GetTarget();
		if (!target) {
			return;
		}
		float Power = 0.0025;
		auto& runtime = Runtime::GetSingleton();

		if (DualCasted())
		{Power *= 2.0;}

		if (GtsManager::GetSingleton().GetFrameNum() % 120 * TimeScale() == 0)
		{
		    auto ShrinkSound = runtime.shrinkSound;
		    float Volume = clamp(0.25, 2.0, get_visual_scale(target)/2);
		    PlaySound(ShrinkSound, target, Volume, 0.0);
			GrowthTremorManager::GetSingleton().CallRumble(target, caster, 0.30);
		}

		if (!Revert(target, Power, Power/2.5)) {
			// Returns false when shrink back is complete
			Dispel();
		}
	}
}
