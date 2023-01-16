#include "managers/GrowthTremorManager.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/highheel.hpp"
#include "magic/effects/common.hpp"
#include "managers/GtsManager.hpp"
#include "data/persistent.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "data/time.hpp"
#include "timer.hpp"
#include "util.hpp"
#include "actorUtils.hpp"
#include "node.hpp"


using namespace RE;
using namespace Gts;

namespace {
	const double LAUNCH_COOLDOWN = 3.0;
	const float LAUNCH_DAMAGE_BASE = 10.0f;
	const float LAUNCH_KNOCKBACK_BASE = 10.0f;
}


namespace Gts {
	SizeManager& SizeManager::GetSingleton() noexcept {
		static SizeManager instance;
		return instance;
	}

	float SizeManager::GetRaceScale(Actor* actor) {
		auto GetNode = find_node(actor, "NPC", false);
		float NodeScale = GetNode->world.scale;
		return GetNode ? NodeScale : 1; // <- not used, causes troubles with quest progression. (Can't reach 1.44 for example when 1.50 is needed.)
	}

	std::string SizeManager::DebugName() {
		return "SizeManager";
	}

	void SizeManager::Update() {
		for (auto actor: find_actors()) {
			// TODO move away from polling
			bool Balance = false; // Toggles Balance Mode for the mod. False = off, true = on.

			float QuestStage = Runtime::GetStage("MainQuest");

			float Gigantism = this->GetEnchantmentBonus(actor)/100;
			float GetLimit = clamp(1.0, 99999999.0, Runtime::GetFloat("sizeLimit")); // Default size limit
			float Persistent_Size = Persistent::GetSingleton().GetData(actor)->bonus_max_size;
			float SelectedFormula = Runtime::GetInt("SelectedSizeFormula");

			float FollowerLimit = Runtime::GetFloat("FollowersSizeLimit");
			float NPCLimit = Runtime::GetFloat("NPCSizeLimit");

			if (SelectedFormula >= 2.0 && actor->formID == 0x14) { // Apply Player Mass-Based max size
				GetLimit = clamp(1.0, 99999999.0, Runtime::GetFloat("MassBasedSizeLimit"));
			} else if (QuestStage > 100 && FollowerLimit > 1 && actor->formID != 0x14 && (Runtime::InFaction(actor, "FollowerFaction") || actor->IsPlayerTeammate())) { // Apply Follower Max Size
				GetLimit = clamp(1.0, 99999999.0, Runtime::GetFloat("FollowersSizeLimit")); // Apply only if Quest is done.
			} else if (QuestStage > 100 && NPCLimit > 1 &&  actor->formID != 0x14 && (!Runtime::InFaction(actor, "FollowerFaction") && !actor->IsPlayerTeammate())) { // Apply Other NPC's max size
				GetLimit = clamp(1.0, 99999999.0, Runtime::GetFloat("NPCSizeLimit"));           // Apply only if Quest is done.
			}

			float RaceScale = (GetRaceScale(actor) * (GetLimit + Persistent_Size)) * (1.0 + Gigantism);
			float TotalLimit = ((GetLimit + Persistent_Size) * (1.0 + Gigantism));

			if (TotalLimit < 1.0) {
				TotalLimit = 1.0;
			}
			if (get_max_scale(actor) < TotalLimit || get_max_scale(actor) > TotalLimit) {
				set_max_scale(actor, TotalLimit);
				//log::info("Current Size Limit of: {} is {}", actor->GetDisplayFullName(), get_max_scale(actor));
			}
		}
	}

	void SizeManager::OnHighheelEquip(const HighheelEquip& evt) {
		float hh_length = evt.hhLength;
		Actor* actor = evt.actor;
		// TODO move away from polling
		if (hh_length > 0 && Runtime::HasPerkTeam(actor, "hhBonus")) { // HH damage bonus start
			auto shoe = evt.shoe;
			float shoe_weight = 1.0; // TODO: Just absorb it into the base
			auto char_weight = actor->GetWeight()/280;
			if (shoe) {
				shoe_weight = shoe->weight/20;
			}
			float expectedhhdamage = 1.5 + shoe_weight + char_weight;
			if (this->GetSizeAttribute(actor, 3) != expectedhhdamage) {
				this->SetSizeAttribute(actor, 1.5 + shoe_weight + char_weight, 3);
				log::info("SizeManager HH Actor {} value: {}", actor->GetDisplayFullName(), this->GetSizeAttribute(actor, 3));
			}
		} else if (hh_length <= 1e-4) {
			if (this->GetSizeAttribute(actor, 3) != 1.0) {
				this->SetSizeAttribute(actor, 1.0, 3);
				log::info("SizeManager HH Actor {} RESET value: {}", actor->GetDisplayFullName(), this->GetSizeAttribute(actor, 3));
			}
		}
	}

	void SizeManager::UnderFootEvent(const UnderFoot& evt) {
		auto giant = evt.giant;
		auto tiny = evt.tiny;
		float force = evt.force;

		float giantSize = get_visual_scale(giant);
		bool hasSMT = Runtime::HasMagicEffect(giant, "SmallMassiveThreat");
		if (hasSMT) {
			giantSize += 4.0;
		}

		float tinySize = get_visual_scale(tiny);

		float movementFactor = 1.0;
		if (evt.footEvent == FootEvent::JumpLand) {
			movementFactor *= 2.0;
		}
		if (giant->IsSneaking()) {
			movementFactor *= 0.5;
		}
		if (giant->IsSneaking()) {
			movementFactor *= 0.5;
		}
		if (giant->IsSprinting()) {
			movementFactor *= 1.5;
		}

		float sizeRatio = giantSize/tinySize * movementFactor;

		if (!SizeManager::IsLaunching(tiny)) {
			if (Runtime::HasPerkTeam(giant, "LaunchPerk")) {
				if (sizeRatio >= 8.0) {
					// Launch
					this->GetLaunchData(tiny).lastLaunchTime = Time::WorldTimeElapsed();
					if (Runtime::HasPerkTeam(giant, "LaunchDamage")) {
						float damage = LAUNCH_DAMAGE_BASE * giantSize * movementFactor * force;
						DamageAV(tiny,ActorValue::kHealth, damage);
					}
					PushActorAway(giant, tiny, LAUNCH_KNOCKBACK_BASE  * giantSize * movementFactor * force);
				}
			}
		}
	}

	void SizeManager::SetEnchantmentBonus(Actor* actor, float amt) {
		if (!actor) {
			return;
		}
		this->GetData(actor).enchantmentBonus = amt;
	}

	float SizeManager::GetEnchantmentBonus(Actor* actor) {
		if (!actor) {
			return 0.0;
		}
		float EB = clamp(0.0, 100.0, this->GetData(actor).enchantmentBonus);
		return EB;
	}

	void SizeManager::ModEnchantmentBonus(Actor* actor, float amt) {
		if (!actor) {
			return;
		}
		this->GetData(actor).enchantmentBonus += amt;
	}

	//=================Size Hunger

	void SizeManager::SetSizeHungerBonus(Actor* actor, float amt) {
		if (!actor) {
			return;
		}
		this->GetData(actor).SizeHungerBonus = amt;
	}

	float SizeManager::GetSizeHungerBonus(Actor* actor) {
		if (!actor) {
			return 0.0;
		}
		float SHB = clamp(0.0, 100.0, this->GetData(actor).SizeHungerBonus);
		return SHB;
	}

	void SizeManager::ModSizeHungerBonus(Actor* actor, float amt) {
		if (!actor) {
			return;
		}
		this->GetData(actor).SizeHungerBonus += amt;
	}

	//==================Growth Spurt

	void SizeManager::SetGrowthSpurt(Actor* actor, float amt) {
		if (!actor) {
			return;
		}
		this->GetData(actor).GrowthSpurt = amt;
	}

	float SizeManager::GetGrowthSpurt(Actor* actor) {
		if (!actor) {
			return 0.0;
		}
		float GS = clamp (0.0, 999999.0, this->GetData(actor).GrowthSpurt);
		return GS;
	}

	void SizeManager::ModGrowthSpurt(Actor* actor, float amt) {
		if (!actor) {
			return;
		}
		this->GetData(actor).GrowthSpurt += amt;
	}

	//================Size-Related Damage
	void SizeManager::SetSizeAttribute(Actor* actor, float amt, float attribute) {
		if (!actor) {
			return;
		}
		auto Persistent = Persistent::GetSingleton().GetData(actor);
		if (!Persistent) {
			return;
		}
		if (attribute == 0) {
			Persistent->NormalDamage = amt;
		} else if (attribute == 1) {
			Persistent->SprintDamage = amt;
		} else if (attribute == 2) {
			Persistent->FallDamage = amt;
		} else if (attribute == 3) {
			Persistent->HHDamage = amt;
		}
	}

	float SizeManager::GetSizeAttribute(Actor* actor, float attribute) {
		if (!actor) {
			return 1.0;
		}
		auto Persistent = Persistent::GetSingleton().GetData(actor);
		if (!Persistent) {
			return 1.0;
		}
		float Normal = clamp (1.0, 999999.0, Persistent->NormalDamage);
		float Sprint = clamp (1.0, 999999.0, Persistent->SprintDamage);
		float Fall = clamp (1.0, 999999.0, Persistent->FallDamage);
		float HH = clamp (1.0, 999999.0, Persistent->HHDamage);
		if (attribute == 0) {
			return Normal;
		} else if (attribute == 1) {
			return Sprint;
		} else if (attribute == 2) {
			return Fall;
		} else if (attribute == 3) {
			return HH;
		}
		return 1.0;
	}

	void SizeManager::ModSizeAttribute(Actor* actor, float amt, float attribute) {
		if (!actor) {
			return;
		}
		auto Persistent = Persistent::GetSingleton().GetData(actor);
		if (!Persistent) {
			return;
		}
		if (attribute == 0) {
			Persistent->NormalDamage += amt;
		} else if (attribute == 1) {
			Persistent->SprintDamage += amt;
		} else if (attribute == 2) {
			Persistent->FallDamage += amt;
		} else if (attribute == 3) {
			Persistent->HHDamage += amt;
		}
	}
	//===============Size-Related Attribute End


	//===============Size-Vulnerability

	void SizeManager::SetSizeVulnerability(Actor* actor, float amt) {
		if (!actor) {
			return;
		}
		auto Persistent = Persistent::GetSingleton().GetData(actor);
		if (!Persistent) {
			return;
		}
		Persistent->SizeVulnerability = amt;
	}

	float SizeManager::GetSizeVulnerability(Actor* actor) {
		if (!actor) {
			return 0.0;
		}
		auto Persistent = Persistent::GetSingleton().GetData(actor);
		if (!Persistent) {
			return 0.0;
		}
		return clamp (0.0, 999999.0, Persistent->SizeVulnerability);
	}

	void SizeManager::ModSizeVulnerability(Actor* actor, float amt) {
		if (!actor) {
			return;
		}
		auto Persistent = Persistent::GetSingleton().GetData(actor);
		if (!Persistent) {
			return;
		}
		Persistent->SizeVulnerability += amt;
	}
	//===============Size-Vulnerability

	//===============Hit Growth

	float SizeManager::GetHitGrowth(Actor* actor) {
		if (!actor) {
			return 0.0;
		}
		auto Persistent = Persistent::GetSingleton().GetData(actor);
		if (!Persistent) {
			return 0.0;
		}
		return Persistent->AllowHitGrowth;
	}

	void SizeManager::SetHitGrowth(Actor* actor, float allow) {
		if (!actor) {
			return;
		}
		auto Persistent = Persistent::GetSingleton().GetData(actor);
		if (!Persistent) {
			return;
		}
		Persistent->AllowHitGrowth = allow;
	}

	//===============Size-Vulnerability

	//===============Balance Mode
	float SizeManager::BalancedMode()
	{
		if (Runtime::GetBool("BalanceMode")) {
			//log::info("Balance Mode True");
			return 2.0;
		} else {
			//log::info("Balance Mode False");
			return 1.0;
		}
	}

	SizeManagerData& SizeManager::GetData(Actor* actor) {
		this->sizeData.try_emplace(actor);
		return this->sizeData.at(actor);
	}

	void SizeManager::Reset() {
		auto caster = PlayerCharacter::GetSingleton();
		if (caster) {
			SetEnchantmentBonus(caster, 0.0);
			SetGrowthSpurt(caster, 0.0);
		}
	}

	LaunchData& SizeManager::GetLaunchData(Actor* actor) {
		this->sizeData.try_emplace(actor);
		return this->sizeData.at(actor);
	}

	bool SizeManager::IsLaunching(Actor* actor) {
		return Time::WorldTimeElapsed() <= (SizeManager::GetSingleton().GetLaunchData(actor).lastLaunchTime + LAUNCH_COOLDOWN)
	}
}
