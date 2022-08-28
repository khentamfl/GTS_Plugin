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
		auto CrushedFoe = GetCaster();
		this->CrushGrowthAmount += 1.0;
		this->ScaleOnCrush = get_visual_scale(CrushedFoe);
	}

	void CrushGrowth::OnUpdate() {
		auto& runtime = Runtime::GetSingleton();
		auto caster = GetCaster();
        auto target = GetTarget();

		if (!caster) {
			log::info("Crush: No caster");
			return;
		}
		if (!target) {
			log::info("Crush: No Target");
			return;
		}
        if (CrushGrowthAmount <= 1.0)
        
        {this->CrushGrowthAmount = 1.0;} // Just to be safe

        float GrowAmount = this->ScaleOnCrush;
        float Rate = 0.00035 * GrowAmount * CrushGrowthAmount;
        if (caster->HasPerk(runtime.AdditionalAbsorption))
		{Rate *= 2.0;}

		float size = get_visual_scale(caster); // We count Enemy as caster: he casts it on us
		float size2 = get_visual_scale(target);
		log::info("Caster {}, target {}, GrowAmount", size, size2, GrowAmount);
        CrushGrow(caster, Rate, 0);
		
	}


    void CrushGrowth::OnFinish() {
        this->CrushGrowthAmount = 1.0;
		this->ScaleOnCrush = 1.0;
    }
}