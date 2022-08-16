#include "magic/effects/slow_grow.hpp"
#include "magic/effects/common.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"

namespace Gts {
	bool SlowGrow::StartEffect(EffectSetting* effect) {
		auto& runtime = Runtime::GetSingleton();
		return effect == runtime.SlowGrowth;
	}

	void SlowGrow::OnUpdate() {
		auto caster = GetCaster();
		if (!caster) {
			return;
		}
		auto target = GetTarget();
		if (!target) {
			return;
		}

		auto& runtime = Runtime::GetSingleton();
		float ProgressionMultiplier = runtime.ProgressionMultiplier->value;
		float casterScale = get_visual_scale(caster);
		float DualCast = 1.0;
		if (IsDualCasting()) {
			DualCast = 2.0;
		}
		mod_target_scale(caster, 0.0010 * ProgressionMultiplier * time_scale());
	}
}
