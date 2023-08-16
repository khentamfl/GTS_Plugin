#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/animation/Utils/CrawlUtils.hpp"
#include "managers/damage/AccurateDamage.hpp"
#include "managers/damage/LaunchActor.hpp"
#include "managers/animation/Crawling.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/CrushManager.hpp"
#include "managers/footstep.hpp"
#include "utils/actorUtils.hpp"
#include "managers/Rumble.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "raycast.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;
using namespace Gts;	

namespace {

	void EnableHandTracking(Actor* giant, CrawlEvent kind, bool decide) {
		if (AllowFeetTracking() && giant->formID == 0x14) {
			auto& sizemanager = SizeManager::GetSingleton();
			if (kind == CrawlEvent::RightHand) {
				sizemanager.SetActionBool(giant, decide, 4.0);
			} else if (kind == CrawlEvent::LeftHand) {
				sizemanager.SetActionBool(giant, decide, 7.0);
			}
		}
	}

    void GTSCrawl_KneeImpact_L(AnimationEventData& data) {
        auto giant = &data.giant;
        float scale = get_visual_scale(giant);
        DoCrawlingFunctions(giant, scale, 1.25, 1.25, CrawlEvent::LeftKnee, "LeftKnee", 20, 18);
	}
	void GTSCrawl_KneeImpact_R(AnimationEventData& data) {
        auto giant = &data.giant;
        float scale = get_visual_scale(giant);
        DoCrawlingFunctions(giant, scale, 1.25, 1.25, CrawlEvent::RightKnee, "RightKnee", 20, 18);
	}
	void GTSCrawl_HandImpact_L(AnimationEventData& data) {
        auto giant = &data.giant;
        float scale = get_visual_scale(giant);
		if (IsTransferingTiny(giant)) {
			return; // Prevent effects from left hand
		}
        DoCrawlingFunctions(giant, scale, 1.0, 1.0, CrawlEvent::LeftHand, "LeftHand", 18, 14);
	}
	void GTSCrawl_HandImpact_R(AnimationEventData& data) {
        auto giant = &data.giant;
        float scale = get_visual_scale(giant);
        DoCrawlingFunctions(giant, scale, 1.0, 1.0, CrawlEvent::RightHand, "RightHand", 18, 14);
		//                                                                               ^    ^ --- Size Damage Radius
        //                                                                             Launch 
		//                                                                             Radius
	}

	void GTSCrawl_Slam_Raise_Arm_R(AnimationEventData& data) {	
		DrainStamina(&data.giant, "StaminaDrain_CrawlStomp", "DestructionBasics", true, 1.0, 1.4);
		EnableHandTracking(&data.giant, CrawlEvent::RightHand, true);
	}

	void GTSCrawl_Slam_Raise_Arm_L(AnimationEventData& data) {	
		DrainStamina(&data.giant, "StaminaDrain_CrawlStomp", "DestructionBasics", true, 1.0, 1.4);
		EnableHandTracking(&data.giant, CrawlEvent::LeftHand, true);
	}
	void GTSCrawl_SlamStrong_Raise_Arm_R(AnimationEventData& data) {
		DrainStamina(&data.giant, "StaminaDrain_CrawlStompStrong", "DestructionBasics", true, 1.10, 2.3);
		EnableHandTracking(&data.giant, CrawlEvent::RightHand, true);
	}
	
	void GTSCrawl_SlamStrong_Raise_Arm_L(AnimationEventData& data) {
		DrainStamina(&data.giant, "StaminaDrain_CrawlStompStrong", "DestructionBasics", true, 1.10, 2.3);
		EnableHandTracking(&data.giant, CrawlEvent::LeftHand, true);
	}

	void GTSCrawl_Slam_Lower_Arm_R(AnimationEventData& data) {}
	void GTSCrawl_SlamStrong_Lower_Arm_R(AnimationEventData& data) {}
	void GTSCrawl_Slam_Lower_Arm_L(AnimationEventData& data) {}
	void GTSCrawl_SlamStrong_Lower_Arm_L(AnimationEventData& data) {}

	void GTSCrawl_Slam_Impact_R(AnimationEventData& data) {
		auto giant = &data.giant;
		float scale = get_visual_scale(giant);
		DoCrawlingFunctions(giant, scale, 1.1, 2.1, CrawlEvent::RightHand, "RightHandRumble", 22, 17);
		DrainStamina(&data.giant, "StaminaDrain_CrawlStomp", "DestructionBasics", false, 1.0, 1.4);
		DrainStamina(&data.giant, "StaminaDrain_CrawlStompStrong", "DestructionBasics", false, 1.10, 2.3);
	}
	void GTSCrawl_Slam_Impact_L(AnimationEventData& data) {
		auto giant = &data.giant;
		float scale = get_visual_scale(giant);
		DoCrawlingFunctions(giant, scale, 1.1, 2.1, CrawlEvent::LeftHand, "LeftHandRumble", 22, 17);
		DrainStamina(&data.giant, "StaminaDrain_CrawlStomp", "DestructionBasics", false, 1.0, 1.4);
		DrainStamina(&data.giant, "StaminaDrain_CrawlStompStrong", "DestructionBasics", false, 1.20, 2.3);
	}


	void GTSCrawl_SlamStrong_Impact_R(AnimationEventData& data) {
		auto giant = &data.giant;
		float scale = get_visual_scale(giant);
		DoCrawlingFunctions(giant, scale, 1.25, 5.2, CrawlEvent::RightHand, "RightHandRumble", 24, 18);
		DrainStamina(&data.giant, "StaminaDrain_CrawlStomp", "DestructionBasics", false, 1.0, 1.4);
		DrainStamina(&data.giant, "StaminaDrain_CrawlStompStrong", "DestructionBasics", false, 1.20, 2.3);
	}
	void GTSCrawl_SlamStrong_Impact_L(AnimationEventData& data) {
		auto giant = &data.giant;
		float scale = get_visual_scale(giant);
		DoCrawlingFunctions(giant, scale, 1.25, 5.2, CrawlEvent::LeftHand, "RightHandRumble", 24, 18);
		DrainStamina(&data.giant, "StaminaDrain_CrawlStomp", "DestructionBasics", false, 1.0, 1.4);
		DrainStamina(&data.giant, "StaminaDrain_CrawlStompStrong", "DestructionBasics", false, 1.20, 2.3);
	}

	void GTSCrawl_Slam_Cam_Off_R(AnimationEventData& data) {
		auto giant = &data.giant;
		EnableHandTracking(&data.giant, CrawlEvent::RightHand, false);
	}
	void GTSCrawl_Slam_Cam_Off_L(AnimationEventData& data) {
		auto giant = &data.giant;
		EnableHandTracking(&data.giant, CrawlEvent::LeftHand, false);
	}

	/////////////////////////////////////////////////////////Swipe Attacks//////////////////////////////////////////

	void TriggerHandCollision_Right(Actor* actor, float power) {
		std::string name = std::format("HandCollide_R_{}", actor->formID);
		auto gianthandle = actor->CreateRefHandle();
		TaskManager::Run(name, [=](auto& progressData) {
			if (!gianthandle) {
				return false;
			}
			auto giant = gianthandle.get().get();
			auto Uarm = find_node(giant, "NPC L Forearm [LLar]");
			auto Arm = find_node(giant, "NPC L Hand [LHnd]");
			if (Uarm && Arm) {
				DoDamageAtPoint(giant, 24, 60.0 * power, Uarm, 100, 0.30, 1.0);
				DoDamageAtPoint(giant, 28, 60.0 * power, Arm, 100, 0.30, 1.0);
				return true;
			}
			return false;
		});
	}

	void TriggerHandCollision_Left(Actor* actor, float power) {
		std::string name = std::format("HandCollide_L_{}", actor->formID);
		auto gianthandle = actor->CreateRefHandle();
		TaskManager::Run(name, [=](auto& progressData) {
			if (!gianthandle) {
				return false;
			}
			auto giant = gianthandle.get().get();
			auto Uarm = find_node(giant, "NPC L Forearm [LLar]");
			auto Arm = find_node(giant, "NPC L Hand [LHnd]");
			if (Uarm && Arm) {
				DoDamageAtPoint(giant, 24, 60.0 * power, Uarm, 10, 0.70, 1.0);
				DoDamageAtPoint(giant, 28, 60.0 * power, Arm, 10, 0.70, 1.0);
				return true;
			}
			return false;
		});
	}
	void DisableHandCollisions(Actor* actor) {
		std::string name = std::format("HandCollide_L_{}", actor->formID);
		std::string name2 = std::format("HandCollide_R_{}", actor->formID);
		TaskManager::Cancel(name);
		TaskManager::Cancel(name2);
	}

	void GTS_Claw_Swipe_On_R(AnimationEventData& data) {
		TriggerHandCollision_Right(&data.giant, 1.0);
	}
	void GTS_Claw_Swipe_On_L(AnimationEventData& data) {
		TriggerHandCollision_Left(&data.giant, 1.0);
	}
	void GTS_Claw_Swipe_Off_R(AnimationEventData& data) {
		DisableHandCollisions(&data.giant);
	}
	void GTS_Claw_Swipe_Off_L(AnimationEventData& data) {
		DisableHandCollisions(&data.giant);
	}

	void GTS_Claw_Swipe_Power_On_R(AnimationEventData& data) {
		TriggerHandCollision_Right(&data.giant, 2.0);
	}
	void GTS_Claw_Swipe_Power_On_L(AnimationEventData& data) {
		TriggerHandCollision_Left(&data.giant, 2.0);
	}
	void GTS_Claw_Swipe_Power_Off_R(AnimationEventData& data) {
		DisableHandCollisions(&data.giant);
	}
	void GTS_Claw_Swipe_Power_Off_L(AnimationEventData& data) {
		DisableHandCollisions(&data.giant);
	}
}

namespace Gts
{
	void AnimationCrawling::RegisterEvents() {
		AnimationManager::RegisterEvent("GTSCrawl_KneeImpact_L", "Crawl", GTSCrawl_KneeImpact_L);
		AnimationManager::RegisterEvent("GTSCrawl_KneeImpact_R", "Crawl", GTSCrawl_KneeImpact_R);
		AnimationManager::RegisterEvent("GTSCrawl_HandImpact_L", "Crawl", GTSCrawl_HandImpact_L);
		AnimationManager::RegisterEvent("GTSCrawl_HandImpact_R", "Crawl", GTSCrawl_HandImpact_R);
		AnimationManager::RegisterEvent("GTSCrawl_Slam_Raise_Arm_R", "Crawl", GTSCrawl_Slam_Raise_Arm_R);
		AnimationManager::RegisterEvent("GTSCrawl_Slam_Raise_Arm_L", "Crawl", GTSCrawl_Slam_Raise_Arm_L);
		AnimationManager::RegisterEvent("GTSCrawl_SlamStrong_Raise_Arm_R", "Crawl", GTSCrawl_SlamStrong_Raise_Arm_R);
		AnimationManager::RegisterEvent("GTSCrawl_SlamStrong_Raise_Arm_L", "Crawl", GTSCrawl_SlamStrong_Raise_Arm_L);
		AnimationManager::RegisterEvent("GTSCrawl_Slam_Lower_Arm_R", "Crawl", GTSCrawl_Slam_Lower_Arm_R);
		AnimationManager::RegisterEvent("GTSCrawl_Slam_Lower_Arm_L", "Crawl", GTSCrawl_Slam_Lower_Arm_L);
		AnimationManager::RegisterEvent("GTSCrawl_SlamStrong_Lower_Arm_R", "Crawl", GTSCrawl_SlamStrong_Lower_Arm_R);
		AnimationManager::RegisterEvent("GTSCrawl_SlamStrong_Lower_Arm_L", "Crawl", GTSCrawl_SlamStrong_Lower_Arm_L);
		AnimationManager::RegisterEvent("GTSCrawl_Slam_Impact_R", "Crawl", GTSCrawl_Slam_Impact_R);
		AnimationManager::RegisterEvent("GTSCrawl_Slam_Impact_L", "Crawl", GTSCrawl_Slam_Impact_L);
		AnimationManager::RegisterEvent("GTSCrawl_SlamStrong_Impact_R", "Crawl", GTSCrawl_SlamStrong_Impact_R);
		AnimationManager::RegisterEvent("GTSCrawl_SlamStrong_Impact_L", "Crawl", GTSCrawl_SlamStrong_Impact_L);

		AnimationManager::RegisterEvent("GTSCrawl_Slam_Cam_Off_L", "Crawl", GTSCrawl_Slam_Cam_Off_L);
		AnimationManager::RegisterEvent("GTSCrawl_Slam_Cam_Off_R", "Crawl", GTSCrawl_Slam_Cam_Off_R);

		AnimationManager::RegisterEvent("GTS_Claw_Swipe_On_R", "Crawl", GTS_Claw_Swipe_On_R);
		AnimationManager::RegisterEvent("GTS_Claw_Swipe_On_L", "Crawl", GTS_Claw_Swipe_On_L);
		AnimationManager::RegisterEvent("GTS_Claw_Swipe_Off_R", "Crawl", GTS_Claw_Swipe_Off_R);
		AnimationManager::RegisterEvent("GTS_Claw_Swipe_Off_L", "Crawl", GTS_Claw_Swipe_Off_L);
		AnimationManager::RegisterEvent("GTS_Claw_Swipe_Power_On_R", "Crawl", GTS_Claw_Swipe_Power_On_R);
		AnimationManager::RegisterEvent("GTS_Claw_Swipe_Power_On_L", "Crawl", GTS_Claw_Swipe_Power_On_L);
		AnimationManager::RegisterEvent("GTS_Claw_Swipe_Power_Off_R", "Crawl", GTS_Claw_Swipe_Power_Off_R);
		AnimationManager::RegisterEvent("GTS_Claw_Swipe_Power_Off_L", "Crawl", GTS_Claw_Swipe_Power_Off_L);
	}

	void AnimationCrawling::RegisterTriggers() {
	}
}