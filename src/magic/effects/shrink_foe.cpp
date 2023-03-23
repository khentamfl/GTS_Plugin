#include "magic/effects/shrink_foe.hpp"
#include "magic/effects/common.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"
#include "data/persistent.hpp"

namespace Gts {
	std::string ShrinkFoe::GetName() {
		return "ShrinkFoe";
	}

	ShrinkFoe::ShrinkFoe(ActiveEffect* effect) : Magic(effect) {
		const float SHRINK_POWER = 1.15; // Power = Shrink Power
		const float SHRINK_EFFIC = 0.18; // Efficiency = size steal efficiency.
		const float SHRINK_AOE_POWER = 1.40;
		const float SHRINK_AOE_EFFIC = 0.22;
		const float SHRINK_AOE_MASTER_POWER = 1.75;
		const float SHRINK_AOE_MASTER_EFFIC = 0.26;
		const float SHRINK_BOLT_POWER = 12.00;
		const float SHRINK_BOLT_EFFIC = 0.05;
		const float SHRINK_STORM_POWER = 24.00;
		const float SHRINK_STORM_EFFIC = 0.12;


		auto base_spell = GetBaseEffect();

		if (base_spell == Runtime::GetMagicEffect("ShrinkEnemy")) {
			this->power = SHRINK_POWER;
			this->efficiency = SHRINK_EFFIC;
		} else if (base_spell == Runtime::GetMagicEffect("ShrinkEnemyAOE")) {
			this->power = SHRINK_AOE_POWER;
			this->efficiency = SHRINK_AOE_EFFIC;
		} else if (base_spell == Runtime::GetMagicEffect("ShrinkEnemyAOEMast")) {
			// ShrinkEnemyAOEMast
			this->power = SHRINK_AOE_MASTER_POWER;
			this->efficiency = SHRINK_AOE_MASTER_EFFIC;
		} else if (base_spell == Runtime::GetMagicEffect("ShrinkBolt")) {
			// ShrinkBolt
			this->power = SHRINK_BOLT_POWER;
			this->efficiency = SHRINK_BOLT_EFFIC;
		} else if (base_spell == Runtime::GetMagicEffect("ShrinkStorm")) {
			// ShrinkBolt
			this->power = SHRINK_STORM_POWER;
			this->efficiency = SHRINK_STORM_EFFIC;
		}
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
		float SizeDifference = 1.0;
		float bonus = 1.0;

		if (this->power >= 18.00) {
			auto& Persist = Persistent::GetSingleton();
			auto actor_data = Persist.GetData(target);
			actor_data->half_life = 0.25; // Faster shrink, less smooth.
			SizeDifference = clamp(1.0, 8.0, (get_target_scale(caster)/get_target_scale(target))/2);
		}

		if (target->IsDead()) {
			bonus = 3.0;
		}

		bool has_smt = Runtime::HasMagicEffect(caster, "SmallMassiveThreat");
		if (target->IsEssential() && Runtime::GetBool("ProtectEssentials")) {
			return; // Disallow shrinking Essentials
		}
		TransferSize(caster, target, IsDualCasting(), this->power * SizeDifference * bonus, this->efficiency, has_smt);
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
