#include "magic/effects/shrink_foe.hpp"
#include "managers/GtsSizeManager.hpp"
#include "magic/effects/common.hpp"
#include "utils/actorUtils.hpp"
#include "data/persistent.hpp"
#include "data/transient.hpp"
#include "data/runtime.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/time.hpp"

namespace {
	void ResetShrinkWeakness(Actor* tiny) {
		auto transient = Transient::GetSingleton().GetData(tiny);
		if (transient) {
			transient->ShrinkWeakness = 1.0;
		}
	}
	void AddShrinkWeakness(Actor* tiny, float value) {
		auto transient = Transient::GetSingleton().GetData(tiny);
		if (transient) {
			transient->ShrinkWeakness += value;
		}
	}
	float GetShrinkWeakness(Actor* tiny) {
		auto transient = Transient::GetSingleton().GetData(tiny);
		if (transient) {
			return transient->ShrinkWeakness;
		}
		return 1.0;
	}

	void DecreaseWeaknessTask(Actor* tiny) {
		float Start = Time::WorldTimeElapsed();
		std::string name = std::format("ShrinkDebuff_{}", tiny->formID);
		ActorHandle tinyhandle = tiny->CreateRefHandle();
		TaskManager::Run(name, [=](auto& progressData) {
			if (!tinyhandle) {
				return false;
			}
			float Finish = Time::WorldTimeElapsed();
			auto tinyref = tinyhandle.get().get();
			float timepassed = Finish - Start;
			if (timepassed >= 2.00) {
				AddShrinkWeakness(tinyref, -0.018 * TimeScale());
				if (GetShrinkWeakness(tinyref) <= 1.0) {
					ResetShrinkWeakness(tinyref);
					return false; // Cancel task
				}
				return true; // end it
			}
			return true;
		});
	}

	void CancelWeaknessTask(Actor* tiny) {
		std::string name = std::format("ShrinkDebuff_{}", tiny->formID);
		TaskManager::Cancel(name);
	}
}


namespace Gts {
	std::string ShrinkFoe::GetName() {
		return "ShrinkFoe";
	}

	ShrinkFoe::ShrinkFoe(ActiveEffect* effect) : Magic(effect) {
		const float SHRINK_POWER = 2.05; // Power = Shrink Power
		const float SHRINK_EFFIC = 0.16; // Efficiency = size steal efficiency.
		const float SHRINK_AOE_POWER = 2.35;
		const float SHRINK_AOE_EFFIC = 0.18;
		const float SHRINK_AOE_MASTER_POWER = 2.70;
		const float SHRINK_AOE_MASTER_EFFIC = 0.20;
		const float SHRINK_BOLT_POWER = 17.50;
		const float SHRINK_BOLT_EFFIC = 0.06;
		const float SHRINK_STORM_POWER = 34.50;
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

	void ShrinkFoe::OnStart() {
		auto caster = GetCaster();
		if (!caster) {
			return;
		}
		auto target = GetTarget();
		if (!target) {
			return;
		}
		float sizediff = get_visual_scale(caster)/get_visual_scale(target);
		if (this->power >= 18.00 && sizediff > 4.0) {
			StaggerActor(target);
		}
		//CancelWeaknessTask(target);
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
		auto& Persist = Persistent::GetSingleton();
		float SizeDifference = 1.0;
		float bonus = 1.0;
		float balancemodebonus = 1.0;
		float shrink = this->power * 2;
		float gainpower = this->efficiency;
		auto actor_data = Persist.GetData(target);

		if (this->power >= 18.00) {
			if (actor_data) {
				actor_data->half_life = 0.25; // Faster shrink, less smooth.
			}
			//SizeDifference = std::clamp((get_visual_scale(caster)/get_visual_scale(target))/2.0f, 1.0f, 2.5f);
			SizeDifference = 1.0 / std::clamp((get_visual_scale(target)), 0.25f, 1.0f);
		} else if (this->power >= 10.0) {
			if (actor_data) {
				actor_data->half_life = 0.50; // Faster shrink, less smooth.
			}
			SizeDifference = 1.0 / std::clamp((get_visual_scale(target)), 0.50f, 1.0f);
		} else {
			if (actor_data) {
				//AddShrinkWeakness(target, 0.0085 * TimeScale());
				actor_data->half_life = 1.0;
			}
		}

		if (target->IsDead()) {
			bonus = 2.5;
			gainpower *= 0.20;
		}

		if (caster->formID == 0x14 && SizeManager::GetSingleton().BalancedMode() == 2.0) { // This is checked only if Balance Mode is enabled. Size requirement is bigger with it.
			balancemodebonus = 2.0;
		}

		float weakness = 1.0;//std::clamp(GetShrinkWeakness(target), 1.0f, 10.0f);
		//log::info("Weakness of {} is {}", target->GetDisplayFullName(), weakness);

		bool has_smt = HasSMT(caster);
		if (IsEssential(target)) {
			return; // Disallow shrinking Essentials
		}
		TransferSize(caster, target, IsDualCasting(), shrink * SizeDifference * bonus * weakness, gainpower * balancemodebonus, has_smt, ShrinkSource::magic);
		
		// 20.11.2023: TO-DO: 
		// 1) Power * 2, cost * 2 too, decreasing time to shrink.
		// 2) Remove stacking debuff thing
		// 3) Reduce shrink penalty at low scales to be 0.5 as a max, instead of being penalty = scale

		if (ShrinkToNothing(caster, target)) {
		}
	}

	void ShrinkFoe::OnFinish() {
		auto Caster = GetCaster();
		auto Target = GetTarget();
		//DecreaseWeaknessTask(Target);
		CastTrackSize(Caster, Target);
	}
}
