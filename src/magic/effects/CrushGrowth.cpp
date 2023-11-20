#include "magic/effects/CrushGrowth.hpp"
#include "magic/effects/common.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"


namespace Gts {
	std::string CrushGrowth::GetName() {
		return "CrushGrowth";
	}

	void CrushGrowth::OnStart() {
		auto CrushedFoe = GetTarget();
		this->CrushGrowthAmount += 1.0;
		this->ScaleOnCrush = get_visual_scale(CrushedFoe);
		//log::info(CrushedFoe->GetDisplayFullName());
	}

	void CrushGrowth::OnUpdate() {
		auto caster = GetCaster();
		auto target = GetTarget();
		float CrushGrowthActivationCount = this->CrushGrowthAmount;

		if (!caster) {
			return;
		}
		if (!target) {
			return;
		}
		if (CrushGrowthActivationCount <= 1.0) {
			CrushGrowthActivationCount = 1.0;
		} // Just to be safe

		float GrowAmount = this->ScaleOnCrush;
		float Rate = 0.00050 * GrowAmount * CrushGrowthActivationCount;
		if (Runtime::HasPerk(caster, "AdditionalGrowth")) {
			Rate *= 2.0;
		}


		float size = get_visual_scale(caster);
		float size2 = get_visual_scale(target);
		//log::info("Caster {}, target {}, GrowAmount {}, CrushGrowth Amount {}", size, size2, GrowAmount, CrushGrowthAmount);
		CrushGrow(caster, 0, Rate);
	}


	void CrushGrowth::OnFinish() {
		this->CrushGrowthAmount = 0.0;
		this->ScaleOnCrush = 1.0;
	}
}
