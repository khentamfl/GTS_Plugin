#include "magic/effects/slow_grow.hpp"
#include "magic/effects/common.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"

namespace Gts {
	std::string MassiveThreat::GetName() {
		return "MassiveThreat";
	}

	void MassiveThreat::OnUpdate() {
		const float BASE_POWER = 0.00035;
		const float DUAL_CAST_BONUS = 2.0;
		auto caster = GetCaster();
		if (!caster) {
			return;
		}
		float CasterScale = get_visual_scale(caster);
        if (CasterScale >= 2.0)
        {Dispel();} // <- Disallow having it when scale is > 2.0
	}
}
