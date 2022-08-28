#include "magic/effects/tracksize.hpp"
#include "magic/effects/common.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"

namespace Gts {
	std::string TrackSize::GetName() {
		return "TrackSize";
	}

	bool TrackSize::StartEffect(EffectSetting* effect) { // NOLINT
		auto& runtime = Runtime::GetSingleton();
		return effect == runtime.GlobalVoreGrowth;
	}

	void TrackSize::OnUpdate() {
		auto caster = GetCaster();
        auto target = GetTarget();
		if (!caster || !target) {
			return;
		}
		float size = get_visual_scale(target);
        ShrinkToNothing(caster, target);
		log::info("Track Size active");
	}
}
