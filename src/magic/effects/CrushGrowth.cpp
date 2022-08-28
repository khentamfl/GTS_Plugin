#include "magic/effects/CrushGrowth.hpp"
#include "magic/effects/common.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"
#include "util.hpp"


namespace Gts {
	std::string CrushGrowth::GetName() {
		return "CrushGrowth";
	}

	bool CrushGrowth::StartEffect(EffectSetting* effect) { // NOLINT
		auto& runtime = Runtime::GetSingleton();
		return effect == runtime.CrushGrowthMGEF;
	}

	void CrushGrowth::OnStart() {
		this->CrushGrowthAmount += 1.0;
	}

	void CrushGrowth::OnUpdate() {
		auto& runtime = Runtime::GetSingleton();
		auto caster = GetCaster();
        auto target = GetTarget();
        
		if (!caster) {
			return;
		}
		if (!target) {
			return;
		}
        if (CrushGrowthAmount <= 0.0)
        
        {this->CrushGrowthAmount = 1.0;} // Just to be safe

        float TargetScale = get_visual_scale(target);
        float Rate = 0.00078 * TargetScale * CrushGrowthAmount;
        if (caster->HasPerk(runtime.AdditionalAbsorption))
		{Rate *= 2.0;}

		float size = get_visual_scale(target);
        CrushGrow(caster, Rate, 0);
		
	}


    void CrushGrowth::OnFinish() {
        this->CrushGrowthAmount = 1.0;
    }
}