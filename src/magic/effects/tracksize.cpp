#include "magic/effects/tracksize.hpp"
#include "magic/effects/common.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"

namespace Gts {
	std::string TrackSize::GetName() {
		return "TrackSize";
	}

	void TrackSize::OnUpdate() {
		auto caster = GetCaster();
		auto target = GetTarget();
		if (!caster) {
			return;
		}
		if (!target) {
			return;
		}
		float size = get_visual_scale(target);
		if (!ShrinkToNothing(caster, target)) {
		}
	}
}
