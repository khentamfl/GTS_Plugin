#include "magic/effects/shrink_foe.hpp"
#include "magic/effects/common.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"

namespace Gts {
	std::string ShrinkFoe::GetName() {
		return "ShrinkFoe";
	}

	ShrinkFoe::ShrinkFoe(ActiveEffect* effect) : Magic(effect) {
		const float SHRINK_POWER = 1.10;
		const float SHRINK_EFFIC = 0.38;
		const float SHRINK_AOE_POWER = 1.25;
		const float SHRINK_AOE_EFFIC = 0.42;
		const float SHRINK_AOE_MASTER_POWER = 1.50;
		const float SHRINK_AOE_MASTER_EFFIC = 0.46;

		auto base_spell = GetBaseEffect();
		auto& runtime = Runtime::GetSingleton();

		if (base_spell == runtime.ShrinkEnemy) {
			this->power = SHRINK_POWER;
			this->efficiency = SHRINK_EFFIC;
		} else if (base_spell == runtime.ShrinkEnemyAOE) {
			this->power = SHRINK_AOE_POWER;
			this->efficiency = SHRINK_AOE_EFFIC;
		} else if (base_spell == runtime.ShrinkEnemyAOEMast) {
			// ShrinkEnemyAOEMast
			this->power = SHRINK_AOE_MASTER_POWER;
			this->efficiency = SHRINK_AOE_MASTER_EFFIC;
		}
	}

	bool ShrinkFoe::StartEffect(EffectSetting* effect) { // NOLINT
		auto& runtime = Runtime::GetSingleton();
		return (effect == runtime.ShrinkEnemy || effect == runtime.ShrinkEnemyAOE || effect == runtime.ShrinkEnemyAOEMast);
	}

	void ShrinkFoe::OnUpdate() {
		auto caster = GetCaster();

		if (!caster) {
			return;
		}
		auto target = GetTarget();
		if (!target) {
			return;
		}

		bool has_smt = caster->HasMagicEffect(Runtime::GetSingleton().SmallMassiveThreat);
		TransferSize(caster, target, IsDualCasting(), this->power, this->efficiency, has_smt);
		if (ShrinkToNothing(caster, target)) {
			Dispel();
		}
	}
}
