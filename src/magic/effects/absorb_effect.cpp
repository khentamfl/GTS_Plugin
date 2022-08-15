#include "magic/effects/absorb_effect.hpp"
#include "magic/effects/common.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"

namespace Gts {
	bool Absorb::StartEffect(EffectSetting* effect) {
		auto& runtime = Runtime::GetSingleton();
		return (effect == runtime.AbsorbMGEF || effect == runtime.TrueAbsorb);
	}

	void Absorb::OnUpdate() {
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
        float targetScale = get_visual_scale(target);
        float SizeDifference = casterScale/targetScale;
        if (effect == runtime.AbsorbMGEF) {
		mod_target_scale(target, -0.0825 * ProgressionMultiplier * SizeDifference);
        mod_target_scale(caster, 0.0025 * ProgressionMultiplier * SizeDifference);
        if (targetScale <= 0.25)
        {caster->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant)->CastSpellImmediate(runtime.FakeCrushSpell, false, target, 1.00f, false, 0.0f, caster);} 
        // ^ Emulate absorption
        }
	    

       else if (effect == runtime.AbsorbMGEF) {
		mod_target_scale(target, -0.0025 * ProgressionMultiplier * SizeDifference);
        mod_target_scale(caster, 0.0005 * ProgressionMultiplier * SizeDifference);}
	
}}
