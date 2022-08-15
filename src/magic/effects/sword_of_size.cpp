#include "magic/effects/sword_of_size.hpp"
#include "magic/effects/common.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"

namespace Gts {
	SwordOfSize::SwordOfSize(ActiveEffect* effect) : Magic(effect) {
	}

	bool SwordOfSize::StartEffect(EffectSetting* effect) {
		auto& runtime = Runtime::GetSingleton();
		if (effect == runtime.SwordEnchant ) {
			return true;
		} else {
			return false;
		}
	}

	void SwordOfSize::OnUpdate() {
		auto caster = GetCaster();
		if (!caster) {
			return;
		}
		auto target = GetTarget();
		if (!target) {
			return;
		}

		transfer_size(caster, target, false, 1.0, 0.38, false);
	}
}
