#include "magic/effects/shrink_back.hpp"
#include "magic/effects/common.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"

namespace Gts {
	std::string ShrinkBack::GetName() {
		return "ShrinkBack";
	}

	bool ShrinkBack::StartEffect(EffectSetting* effect) { // NOLINT
		auto& runtime = Runtime::GetSingleton();
		return effect == runtime.ShrinkBack;
	}

	void ShrinkBack::OnStart() {
		Actor* caster = GetCaster();
		if (!caster) {
			return;
		}
		auto& runtime = Runtime::GetSingleton();
		auto ShrinkSound = runtime.shrinkSound;
		float Volume = clamp(0.50, 1.0, get_visual_scale(caster));
		PlaySound(ShrinkSound, caster, Volume);
	}

	void ShrinkBack::OnUpdate() {
		auto caster = GetCaster();
		if (!caster) {
			return;
		}
		float Power = 0.0025;
		auto& runtime = Runtime::GetSingleton();

		if (DualCasted())
		{Power *= 2.0;}

		if (!Revert(caster, Power, Power/2.5)) {
			// Returns false when shrink back is complete
			Dispel();
		}
	}
}
