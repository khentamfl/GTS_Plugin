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
		const float SHRINK_POWER = 1.15;
		const float SHRINK_EFFIC = 0.38;
		const float SHRINK_AOE_POWER = 1.40;
		const float SHRINK_AOE_EFFIC = 0.42;
		const float SHRINK_AOE_MASTER_POWER = 1.75;
		const float SHRINK_AOE_MASTER_EFFIC = 0.46;
		const float SHRINK_BOLT_POWER = 12.90;
		const float SHRINK_BOLT_EFFIC = 1.35;

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
		else if (base_spell == runtime.ShrinkBolt) {
			// ShrinkEnemyAOEMast
			this->power = SHRINK_BOLT_POWER;
			this->efficiency = SHRINK_BOLT_EFFIC;
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

	void ShrinkFoe::OnFinish() {
		auto Caster = GetCaster();
		auto Target = GetTarget();
		CastTrackSize(Caster, Target);
	}
}
