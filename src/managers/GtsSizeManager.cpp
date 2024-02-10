#include "managers/GtsSizeManager.hpp"
#include "managers/CrushManager.hpp"
#include "managers/GtsManager.hpp"
#include "managers/Attributes.hpp"
#include "managers/highheel.hpp"
#include "managers/InputManager.hpp"
#include "managers/Rumble.hpp"
#include "magic/effects/common.hpp"
#include "utils/actorUtils.hpp"
#include "data/persistent.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "data/time.hpp"
#include "profiler.hpp"
#include "timer.hpp"
#include "node.hpp"

#include <random>


using namespace Gts;
using namespace RE;
using namespace REL;
using namespace SKSE;

namespace {
	const double LAUNCH_COOLDOWN = 0.8f;
	const double DAMAGE_COOLDOWN = 2.0f;
	const double HANDDAMAGE_COOLDOWN = 0.6f;
	const double THIGHDAMAGE_COOLDOWN = 1.2f;
	const double SCARE_COOLDOWN = 6.0f;
	const float LAUNCH_DAMAGE_BASE = 1.0f;
	const float LAUNCH_KNOCKBACK_BASE = 0.02f;

	float Calculate_Halflife(CameraTracking Bone) {
		if (Bone == CameraTracking::Thigh_Crush) { // Thigh Crushing
			return 0.15;
		} else if (Bone == CameraTracking::VoreHand_Right || Bone == CameraTracking::Hand_Left || Bone == CameraTracking::Hand_Right) { // Voring / using hands
			return 0.10;
		} else if (Bone == CameraTracking::ObjectA || Bone == CameraTracking::ObjectB) { // pretty much vore/ hands too
			return 0.10;
		} else if (Bone == CameraTracking::R_Foot || Bone == CameraTracking::L_Foot) { // Feet
			return 0.08;
		} else if (Bone == CameraTracking::Butt || Bone == CameraTracking::Mid_Butt_Legs) { // Butt
			return 0.08;
		} else if (Bone == CameraTracking::Breasts_02) { // Breasts
			return 0.10;
		} else if (Bone == CameraTracking::Knees) { // Knees
			return 0.10;
		} else if (Bone == CameraTracking::Finger_Right || Bone == CameraTracking::Finger_Left) {
			return 0.08;
		} else {
			return 0.05;
		}
	}

	float get_endless_height(Actor* giant) {
		float endless = 0.0;
		if (Runtime::HasPerk(giant, "ColossalGrowth")) {
			endless = 99999999.0;
		}
		return endless;
	}
}

namespace Gts {
	SizeManager& SizeManager::GetSingleton() noexcept {
		static SizeManager instance;
		return instance;
	}

	std::string SizeManager::DebugName() {
		return "SizeManager";
	}

	void SizeManager::Update() {
		auto profiler = Profilers::Profile("SizeManager: Update");
		for (auto actor: find_actors()) {
			// 2023: TODO: move away from polling
			float Endless = 0.0;
			if (actor->formID == 0x14) {
				Endless = get_endless_height(actor);
			}
			float NaturalScale = get_neutral_scale(actor);
			float Gigantism = Ench_Aspect_GetPower(actor);

			float QuestStage = Runtime::GetStage("MainQuest");
			auto Persistent = Persistent::GetSingleton().GetData(actor);
			
			float GetLimit = clamp(NaturalScale, 99999999.0, NaturalScale + ((Runtime::GetFloat("sizeLimit") - 1.0) * NaturalScale)); // Default size limit
			
			float Persistent_Size = 0.0;
			float SelectedFormula = Runtime::GetInt("SelectedSizeFormula");

			float FollowerLimit = Runtime::GetFloat("FollowersSizeLimit"); // 0 by default
			float NPCLimit = Runtime::GetFloat("NPCSizeLimit"); // 0 by default

			if (Persistent) {
				Persistent_Size = Persistent->bonus_max_size;
			}

			if (SelectedFormula >= 1.0 && actor->formID == 0x14) { // Apply Player Mass-Based max size
				float low_limit = get_endless_height(actor);
				if (low_limit < 2) {
					low_limit = Runtime::GetFloat("sizeLimit");
				}
				GetLimit = clamp(NaturalScale, low_limit, NaturalScale + (Runtime::GetFloat("GtsMassBasedSize") * NaturalScale));
			} else if (QuestStage > 100 && FollowerLimit > 0.0 && FollowerLimit != 1.0 && actor->formID != 0x14 && IsTeammate(actor)) { // Apply Follower Max Size
				GetLimit = clamp(NaturalScale * FollowerLimit, 99999999.0, NaturalScale + ((Runtime::GetFloat("FollowersSizeLimit") - 1.0) * NaturalScale)); // Apply only if Quest is done.
			} else if (QuestStage > 100 && NPCLimit > 0.0 && NPCLimit != 1.0 && actor->formID != 0x14 && !IsTeammate(actor)) { // Apply Other NPC's max size
				GetLimit = clamp(NaturalScale * NPCLimit, 99999999.0, NaturalScale + ((Runtime::GetFloat("NPCSizeLimit") - 1.0) * NaturalScale));       // Apply only if Quest is done.
			}

			float TotalLimit = ((GetLimit + Persistent_Size) * (1.0 + Gigantism));

			if (get_max_scale(actor) < TotalLimit + Endless || get_max_scale(actor) > TotalLimit + Endless) {
				set_max_scale(actor, TotalLimit);
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
		float EB = clamp(0.0, 1000.0, this->GetData(actor).enchantmentBonus);
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
		float SHB = clamp(0.0, 1000.0, this->GetData(actor).SizeHungerBonus);
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
		this->GetData(actor).GrowthSpurtSize = amt;
	}

	float SizeManager::GetGrowthSpurt(Actor* actor) {
		if (!actor) {
			return 0.0;
		}
		float GS = clamp (0.0, 999999.0, this->GetData(actor).GrowthSpurtSize);
		return GS;
	}

	void SizeManager::ModGrowthSpurt(Actor* actor, float amt) {
		if (!actor) {
			return;
		}
		this->GetData(actor).GrowthSpurtSize += amt;
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
			return GetHighHeelsBonusDamage(actor);
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
		return clamp(0.0, 3.0, Persistent->SizeVulnerability);
	}

	void SizeManager::ModSizeVulnerability(Actor* actor, float amt) {
		if (!actor) {
			return;
		}
		auto Persistent = Persistent::GetSingleton().GetData(actor);
		if (!Persistent) {
			return;
		}
		if (Persistent->SizeVulnerability < 3) {
			Persistent->SizeVulnerability += amt;
		}
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

	//===============Camera Stuff
	void SizeManager::SetTrackedBone(Actor* actor, bool enable, CameraTracking Bone) {
		if (!enable) {
			Bone = CameraTracking::None;
		}
		SetCameraHalflife(actor, Bone);
		SetCameraOverride(actor, enable);
		this->GetData(actor).TrackedBone = Bone;
	}

	CameraTracking SizeManager::GetTrackedBone(Actor* actor) {
		return this->GetData(actor).TrackedBone;
	}


	//==============Half life stuff
	void SizeManager::SetCameraHalflife(Actor* actor, CameraTracking Bone) {
		this->GetData(actor).Camera_HalfLife = Calculate_Halflife(Bone);
	}

	float SizeManager::GetCameraHalflife(Actor* actor) {
		return this->GetData(actor).Camera_HalfLife;
	}
	//

	//===============Balance Mode
	float SizeManager::BalancedMode()
	{
		if (Runtime::GetBool("BalanceMode")) {
			return 2.0;
		} else {
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
		
		TaskManager::CancelAllTasks(); // just in case, to avoid CTD
		//this->data.clear();
	}

	LaunchData& SizeManager::GetLaunchData(Actor* actor) {
		this->launchData.try_emplace(actor);
		return this->launchData.at(actor);
	}

	DamageData& SizeManager::GetDamageData(Actor* actor) {
		this->DamageData.try_emplace(actor);
		return this->DamageData.at(actor);
	}

	bool SizeManager::IsLaunching(Actor* actor) {
		return Time::WorldTimeElapsed() <= (SizeManager::GetSingleton().GetLaunchData(actor).lastLaunchTime + LAUNCH_COOLDOWN);
	}

	bool SizeManager::IsDamaging(Actor* actor) {
		return Time::WorldTimeElapsed() <= (SizeManager::GetSingleton().GetDamageData(actor).lastDamageTime + DAMAGE_COOLDOWN);
	}

	bool SizeManager::IsHandDamaging(Actor* actor) {
		return Time::WorldTimeElapsed() <= (SizeManager::GetSingleton().GetDamageData(actor).lastHandDamageTime + HANDDAMAGE_COOLDOWN);
	}

	bool SizeManager::IsThighDamaging(Actor* actor) {
		return Time::WorldTimeElapsed() <= (SizeManager::GetSingleton().GetDamageData(actor).lastThighDamageTime + THIGHDAMAGE_COOLDOWN);
	}

	bool SizeManager::IsBeingScared(Actor* actor) {
		return Time::WorldTimeElapsed() <= (SizeManager::GetSingleton().GetDamageData(actor).lastScareTime + SCARE_COOLDOWN);
	}

	bool SizeManager::GetPreciseDamage() {
		auto result = Runtime::GetBool("PreciseDamage");
		return result;
	}
}