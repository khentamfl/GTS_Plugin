#include "magic/effects/sword_of_size.h"
#include "magic/effects/common.h"
#include "magic/magic.h"
#include "scale/scale.h"
#include "data/runtime.h"

namespace Gts {
	SwordOfSize::SwordOfSize(ActiveEffect* effect) {
		this->activeEffect = effect;
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
		if (!targer) {
			return;
		}

		transfer_size(caster, target, false, 1.0, 0.38, false);
	}
}
