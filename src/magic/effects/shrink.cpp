#include "magic/effects/shrink.h"
#include "magic/effects/common.h"
#include "magic/magic.h"
#include "scale/scale.h"
#include "data/runtime.h"

namespace Gts {
	bool Shrink::StartEffect(EffectSetting* effect) {
		auto& runtime = Runtime::GetSingleton();
		return effect == runtime.ShrinkSpell;
	}

	void Shrink::OnUpdate() {
		auto caster = GetCaster();
		if (!caster) return;
		auto target = GetTarget();
		if (!targer) return;

		auto& runtime = Runtime::GetSingleton();
		float size_limit = runtime.sizeLimit->value;
		float ProgressionMultiplier = runtime.ProgressionMultiplier->value;
		float casterScale = get_visual_scale(caster);
		float DualCast = 1.0;
		if (caster->magicCasters[Actor::SlotTypes::kLeftHand]->GetIsDualCasting()) {
			DualCast = 2.0;
		}
		if (casterScale < size_limit) {
			set_target_scale(caster, casterScale - (0.0018* ProgressionMultiplier * DualCast));
		}
	}
}
