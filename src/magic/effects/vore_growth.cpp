#include "magic/effects/vore_growth.hpp"
#include "magic/effects/common.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"

namespace Gts {
	std::string VoreGrowth::GetName() {
		return "VoreGrowth";
	}

	bool VoreGrowth::StartEffect(EffectSetting* effect) { // NOLINT
		auto& runtime = Runtime::GetSingleton();
		return effect == runtime.GlobalVoreGrowth;
	}

	void VoreGrowth::OnUpdate() {
		float BASE_POWER = 0.002475;
		auto& runtime = Runtime::GetSingleton();
		auto caster = GetCaster();
		auto target = GetTarget();
		if (!caster || !target) {
			return;
		}
		float targetscale = get_visual_scale(target);
		BASE_POWER *= targetscale;
		if (caster->HasPerk(runtime.AdditionalAbsorption))
		{BASE_POWER *= 2.0;}
		Grow(caster, 0.0, BASE_POWER);
		log::info("Vore Growth active");
	}
}
