#include "magic/effects/shrink.hpp"
#include "magic/effects/common.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"

namespace Gts {
	bool Shrink::StartEffect(EffectSetting* effect) {
		auto& runtime = Runtime::GetSingleton();
		return effect == runtime.ShrinkSpell;
	}

	void Shrink::OnUpdate() {
		auto caster = GetCaster();
		if (!caster) {
			return;
		}
		auto target = GetTarget();
		if (!targer) {
			return;
		}

		auto& runtime = Runtime::GetSingleton();
		float size_limit = runtime.sizeLimit->value;
		float ProgressionMultiplier = runtime.ProgressionMultiplier->value;
		float casterScale = get_visual_scale(caster);
		float DualCast = 1.0;
		if (IsDuelCasting()) {
			DualCast = 2.0;
		}
		if (casterScale < size_limit) {
			mod_target_scale(caster, -0.0018* ProgressionMultiplier * DualCast);
		}
	}
}
