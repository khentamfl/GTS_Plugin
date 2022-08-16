#include "magic/effects/shrink_back.hpp"
#include "magic/effects/common.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"

namespace Gts {
	bool ShrinkBack::StartEffect(EffectSetting* effect) { // NOLINT
		auto& runtime = Runtime::GetSingleton();
		return effect == runtime.ShrinkBack;
	}

	void ShrinkBack::OnUpdate() {
		auto caster = GetCaster();
		if (!caster) {
			return;
		}

		if (!Revert(caster, 0.0025, 0.0010)) {
			// Returns false when shrink back is complete
			Dispel();
		}
	}
}
