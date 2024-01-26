#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/animation/Utils/CrawlUtils.hpp"
#include "managers/damage/CollisionDamage.hpp"
#include "managers/damage/LaunchActor.hpp"
#include "managers/animation/Sneak_Swipes.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/CrushManager.hpp"
#include "managers/InputManager.hpp"
#include "managers/footstep.hpp"
#include "utils/actorUtils.hpp"
#include "managers/Rumble.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "rays/raycast.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace {
	void TriggerHandCollision_Right(Actor* actor, float power, float crush, float pushpower) {
		std::string name = std::format("HandCollide_R_{}", actor->formID);
		auto gianthandle = actor->CreateRefHandle();
		TaskManager::Run(name, [=](auto& progressData) {
			if (!gianthandle) {
				return false;
			}
			auto giant = gianthandle.get().get();
			auto Uarm = find_node(giant, "NPC R Forearm [RLar]");
			auto Arm = find_node(giant, "NPC R Hand [RHnd]");
			if (Uarm) {
				DoDamageAtPoint_Cooldown(giant, 17, 80.0 * power, Uarm, 10, 0.30, crush, pushpower, DamageSource::HandSwipeRight);
			}
			if (Arm) {
				DoDamageAtPoint_Cooldown(giant, 19, 80.0 * power, Arm, 10, 0.30, crush, pushpower, DamageSource::HandSwipeRight);
			}
			return true;
		});
	}

	void TriggerHandCollision_Left(Actor* actor, float power, float crush, float pushpower) {
		std::string name = std::format("HandCollide_L_{}", actor->formID);
		auto gianthandle = actor->CreateRefHandle();
		TaskManager::Run(name, [=](auto& progressData) {
			if (!gianthandle) {
				return false;
			}
			auto giant = gianthandle.get().get();
			auto Uarm = find_node(giant, "NPC L Forearm [LLar]");
			auto Arm = find_node(giant, "NPC L Hand [LHnd]");
			if (Uarm) {
				DoDamageAtPoint_Cooldown(giant, 17, 80.0 * power, Uarm, 10, 0.30, crush, pushpower, DamageSource::HandSwipeLeft);
			}
			if (Arm) {
				DoDamageAtPoint_Cooldown(giant, 19, 80.0 * power, Arm, 10, 0.30, crush, pushpower, DamageSource::HandSwipeLeft);
			}
			return true;
		});
	}
	void DisableHandCollisions(Actor* actor) {
		std::string name = std::format("HandCollide_L_{}", actor->formID);
		std::string name2 = std::format("HandCollide_R_{}", actor->formID);
		TaskManager::Cancel(name);
		TaskManager::Cancel(name2);
	}

	void GTS_Crawl_Swipe_ArmSfx_Start(AnimationEventData& data) {
	}
	void GTS_Crawl_Swipe_ArmSfx_End(AnimationEventData& data) {
	}

	void GTS_Crawl_Swipe_On_R(AnimationEventData& data) {
		TriggerHandCollision_Right(&data.giant, 1.4, 1.6, 0.75);
		DrainStamina(&data.giant, "StaminaDrain_CrawlSwipe", "DestructionBasics", true, 4.0);
	}
	void GTS_Crawl_Swipe_On_L(AnimationEventData& data) {
		TriggerHandCollision_Left(&data.giant, 1.4, 1.6, 0.75);
		DrainStamina(&data.giant, "StaminaDrain_CrawlSwipe", "DestructionBasics", true, 4.0);
	}
	void GTS_Crawl_Swipe_Off_R(AnimationEventData& data) {
		DisableHandCollisions(&data.giant);
		DrainStamina(&data.giant, "StaminaDrain_CrawlSwipe", "DestructionBasics", false, 4.0);
	}
	void GTS_Crawl_Swipe_Off_L(AnimationEventData& data) {
		DisableHandCollisions(&data.giant);
		DrainStamina(&data.giant, "StaminaDrain_CrawlSwipe", "DestructionBasics", false, 4.0);
	}

	void GTS_Crawl_Swipe_Power_On_R(AnimationEventData& data) {
		TriggerHandCollision_Right(&data.giant, 3.6, 1.3, 1.4);
		DrainStamina(&data.giant, "StaminaDrain_CrawlSwipeStrong", "DestructionBasics", true, 10.0);
	}
	void GTS_Crawl_Swipe_Power_On_L(AnimationEventData& data) {
		TriggerHandCollision_Left(&data.giant, 3.6, 1.3, 1.4);
		DrainStamina(&data.giant, "StaminaDrain_CrawlSwipeStrong", "DestructionBasics", true, 10.0);
	}
	void GTS_Crawl_Swipe_Power_Off_R(AnimationEventData& data) {
		DisableHandCollisions(&data.giant);
		DrainStamina(&data.giant, "StaminaDrain_CrawlSwipeStrong", "DestructionBasics", false, 10.0);
	}
	void GTS_Crawl_Swipe_Power_Off_L(AnimationEventData& data) {
		DisableHandCollisions(&data.giant);
		DrainStamina(&data.giant, "StaminaDrain_CrawlSwipeStrong", "DestructionBasics", false, 10.0);
	}
}

namespace Gts {
    
    void Animation_SneakSwipes::RegisterEvents() {
		//AnimationManager::RegisterEvent("GTSstompimpactR", "Stomp", GTSstompimpactR);
		//AnimationManager::RegisterEvent("GTSstompimpactL", "Stomp", GTSstompimpactL);
    }
}
