#include "magic/effects/smallmassivethreat.hpp"
#include "magic/effects/common.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"

namespace Gts {

	std::string SmallMassiveThreat::GetName() {
		return "SmallMassiveThreat";
	}

	void SmallMassiveThreat::OnUpdate() {
		const float BASE_POWER = 0.00035;
		const float DUAL_CAST_BONUS = 2.0;
		auto caster = GetCaster();
		if (!caster) {
			return;
		}
		inline static Timer warningtimer = Timer(3.0);
		float CasterScale = get_target_scale(caster);
		if (CasterScale >= 1.50) {
			mod_target_scale(caster, -0.035);
			if (warningtimer.ShouldRun()) {
				Notify("Im getting too big, it becomes hard to handle such power.");
			}
		} // <- Disallow having it when scale is > 2.0
	}
}
