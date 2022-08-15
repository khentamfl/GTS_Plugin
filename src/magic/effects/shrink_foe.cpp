#include "magic/effects/shrink_foe.h"
#include "magic/effects/common.h"
#include "magic/magic.h"
#include "scale/scale.h"
#include "data/runtime.h"

namespace Gts {
	ShrinkFoe::ShrinkFoe(ActiveEffect* effect) {
		this->activeEffect = effect;
		this->effectSetting = effect->GetBaseEffect();
		auto& runtime = Runtime::GetSingleton();

		if (this->effectSetting == runtime.explosiveGrowth1) {
			this->power = 1.0;
			this->efficiency = 0.34;
		} else if (this->effectSetting == runtime.explosiveGrowth2) {
			this->power = 1.12;
			this->efficiency = 0.36;
		} else {
			// ShrinkEnemyAOEMast
			this->power = 1.35;
			this->efficiency = 0.40;
		}
	}

	bool ShrinkFoe::StartEffect(EffectSetting* effect) {
		auto& runtime = Runtime::GetSingleton();
		if (effect == runtime.ShrinkEnemy || effect == runtime.ShrinkEnemyAOE || effect == runtime.ShrinkEnemyAOEMast) {
			return true;
		} else {
			return false;
		}
	}

	void ShrinkFoe::OnUpdate() {
		auto caster = GetCaster();
		if (!caster) {
			return;
		}
		auto target = GetTarget();
		if (!targer) {
			return;
		}
		bool smallMassiveThreat = caster->HasMagicEffect(Runtime::GetSingleton().smallMassiveThreat);
		transfer_size(caster, target, IsDuelCasting(), this->power, this->efficiency, smallMassiveThreat);
	}
}
