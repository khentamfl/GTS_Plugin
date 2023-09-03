#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/animation/Utils/CrawlUtils.hpp"
#include "managers/damage/AccurateDamage.hpp"
#include "managers/damage/LaunchActor.hpp"
#include "managers/animation/Crawling.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/CrushManager.hpp"
#include "managers/InputManager.hpp"
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
        DoCrawlingFunctions(giant, scale, 1.25, 1.25, CrawlEvent::LeftKnee, "LeftKnee", 20, 18, 1.25, DamageSource::Knee);
	}
	void GTSCrawl_KneeImpact_R(AnimationEventData& data) {
        auto giant = &data.giant;
        float scale = get_visual_scale(giant);
        DoCrawlingFunctions(giant, scale, 1.25, 1.25, CrawlEvent::RightKnee, "RightKnee", 20, 18, 1.25, DamageSource::Knee);
	}
	void GTSCrawl_HandImpact_L(AnimationEventData& data) {
        auto giant = &data.giant;
        float scale = get_visual_scale(giant);
		if (IsTransferingTiny(giant)) {
			return; // Prevent effects from left hand
		}
        DoCrawlingFunctions(giant, scale, 1.0, 1.0, CrawlEvent::LeftHand, "LeftHand", 18, 14, 1.25, DamageSource::HandCrawl);
	}
	void GTSCrawl_HandImpact_R(AnimationEventData& data) {
        auto giant = &data.giant;
        float scale = get_visual_scale(giant);
        DoCrawlingFunctions(giant, scale, 1.0, 1.0, CrawlEvent::RightHand, "RightHand", 18, 14, 1.25, DamageSource::HandCrawl);
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
		DoCrawlingFunctions(giant, scale, 1.1, 2.1, CrawlEvent::RightHand, "RightHandRumble", 22, 17, 1.15, DamageSource::HandSlam);
		DrainStamina(&data.giant, "StaminaDrain_CrawlStomp", "DestructionBasics", false, 1.0, 1.4);
		DrainStamina(&data.giant, "StaminaDrain_CrawlStompStrong", "DestructionBasics", false, 1.10, 2.3);
	}
	void GTSCrawl_Slam_Impact_L(AnimationEventData& data) {
		auto giant = &data.giant;
		float scale = get_visual_scale(giant);
		DoCrawlingFunctions(giant, scale, 1.1, 2.1, CrawlEvent::LeftHand, "LeftHandRumble", 22, 17, 1.15, DamageSource::HandSlam);
		DrainStamina(&data.giant, "StaminaDrain_CrawlStomp", "DestructionBasics", false, 1.0, 1.4);
		DrainStamina(&data.giant, "StaminaDrain_CrawlStompStrong", "DestructionBasics", false, 1.20, 2.3);
	}


	void GTSCrawl_SlamStrong_Impact_R(AnimationEventData& data) {
		auto giant = &data.giant;
		float scale = get_visual_scale(giant);
		DoCrawlingFunctions(giant, scale, 1.25, 5.2, CrawlEvent::RightHand, "RightHandRumble", 24, 18, 1.0, DamageSource::HandSlam);
		DrainStamina(&data.giant, "StaminaDrain_CrawlStomp", "DestructionBasics", false, 1.0, 1.4);
		DrainStamina(&data.giant, "StaminaDrain_CrawlStompStrong", "DestructionBasics", false, 1.20, 2.3);
	}
	void GTSCrawl_SlamStrong_Impact_L(AnimationEventData& data) {
		auto giant = &data.giant;
		float scale = get_visual_scale(giant);
		DoCrawlingFunctions(giant, scale, 1.25, 5.2, CrawlEvent::LeftHand, "RightHandRumble", 24, 18, 1.0, DamageSource::HandSlam);
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
				DoDamageAtPoint_Cooldown(giant, 17, 80.0 * power, Uarm, 10, 0.30, crush, pushpower);
			}
			if (Arm) {
				DoDamageAtPoint_Cooldown(giant, 19, 80.0 * power, Arm, 10, 0.30, crush, pushpower);
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
				DoDamageAtPoint_Cooldown(giant, 17, 80.0 * power, Uarm, 10, 0.30, crush, pushpower);
			}
			if (Arm) {
				DoDamageAtPoint_Cooldown(giant, 19, 80.0 * power, Arm, 10, 0.30, crush, pushpower);
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

	void DisableHandTrackingTask(Actor* giant) { // Used to disable camera with some delay
		std::string name = std::format("CameraOff_{}", giant->formID);
        auto gianthandle = giant->CreateRefHandle();
        auto FrameA = Time::FramesElapsed();
        TaskManager::Run(name, [=](auto& progressData) {
			if (!gianthandle) {
				return false;
			}
			auto giantref = gianthandle.get().get();
			auto FrameB = Time::FramesElapsed() - FrameA;
			if (FrameB <= 60.0/GetAnimationSlowdown(giantref)) {
				return true;
			}
	
            EnableHandTracking(giantref, CrawlEvent::RightHand, false);
			EnableHandTracking(giantref, CrawlEvent::LeftHand, false);

			return false;
		});
	}

	void GTS_Crawl_Swipe_ArmSfx_Start(AnimationEventData& data) {}
	void GTS_Crawl_Swipe_ArmSfx_End(AnimationEventData& data) {}

	void GTS_Crawl_Swipe_On_R(AnimationEventData& data) {
		TriggerHandCollision_Right(&data.giant, 1.4, 1.6, 0.75);
		DrainStamina(&data.giant, "StaminaDrain_CrawlSwipe", "DestructionBasics", true, 2.0, 4.0);
	}
	void GTS_Crawl_Swipe_On_L(AnimationEventData& data) {
		TriggerHandCollision_Left(&data.giant, 1.4, 1.6, 0.75);
		DrainStamina(&data.giant, "StaminaDrain_CrawlSwipe", "DestructionBasics", true, 2.0, 4.0);
	}
	void GTS_Crawl_Swipe_Off_R(AnimationEventData& data) {
		DisableHandCollisions(&data.giant);
		DrainStamina(&data.giant, "StaminaDrain_CrawlSwipe", "DestructionBasics", false, 2.0, 4.0);
	}
	void GTS_Crawl_Swipe_Off_L(AnimationEventData& data) {
		DisableHandCollisions(&data.giant);
		DrainStamina(&data.giant, "StaminaDrain_CrawlSwipe", "DestructionBasics", false, 2.0, 4.0);
	}

	void GTS_Crawl_Swipe_Power_On_R(AnimationEventData& data) {
		TriggerHandCollision_Right(&data.giant, 3.6, 1.3, 1.4);
		DrainStamina(&data.giant, "StaminaDrain_CrawlSwipeStrong", "DestructionBasics", true, 3.0, 10.0);
	}
	void GTS_Crawl_Swipe_Power_On_L(AnimationEventData& data) {
		TriggerHandCollision_Left(&data.giant, 3.6, 1.3, 1.4);
		DrainStamina(&data.giant, "StaminaDrain_CrawlSwipeStrong", "DestructionBasics", true, 3.0, 10.0);
	}
	void GTS_Crawl_Swipe_Power_Off_R(AnimationEventData& data) {
		DisableHandCollisions(&data.giant);
		DrainStamina(&data.giant, "StaminaDrain_CrawlSwipeStrong", "DestructionBasics", false, 3.0, 10.0);
	}
	void GTS_Crawl_Swipe_Power_Off_L(AnimationEventData& data) {
		DisableHandCollisions(&data.giant);
		DrainStamina(&data.giant, "StaminaDrain_CrawlSwipeStrong", "DestructionBasics", false, 3.0, 10.0);
	}

	void LightSwipeLeftEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		if (IsCrawling(player)) {
			float WasteStamina = 25.0 * GetWasteMult(player);
			if (GetAV(player, ActorValue::kStamina) > WasteStamina) {
				//player->SetGraphVariableBool("GTS_Busy", true);
				AnimationManager::StartAnim("SwipeLight_Left", player);
			} else {
				TiredSound(player, "You're too tired for hand swipe");
			}
		}
	}
	void LightSwipeRightEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		if (IsCrawling(player)) {
			float WasteStamina = 25.0 * GetWasteMult(player);
			if (GetAV(player, ActorValue::kStamina) > WasteStamina) {
				//player->SetGraphVariableBool("GTS_Busy", true);
				AnimationManager::StartAnim("SwipeLight_Right", player);
			} else {
				TiredSound(player, "You're too tired for hand swipe");
			}
		}
	}

	void HeavySwipeLeftEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		if (IsCrawling(player)) {
			float WasteStamina = 70.0 * GetWasteMult(player);
			if (GetAV(player, ActorValue::kStamina) > WasteStamina) {
				//player->SetGraphVariableBool("GTS_Busy", true);
				AnimationManager::StartAnim("SwipeHeavy_Left", player);
			} else {
				TiredSound(player, "You're too tired for hand swipe");
			}
		}
	}
	void HeavySwipeRightEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		if (IsCrawling(player)) {
			float WasteStamina = 70.0 * GetWasteMult(player);
			if (GetAV(player, ActorValue::kStamina) > WasteStamina) {
				//player->SetGraphVariableBool("GTS_Busy", true);
				AnimationManager::StartAnim("SwipeHeavy_Right", player);
			} else {
				TiredSound(player, "You're too tired for hand swipe");
			}
		}
	}
}

namespace Gts
{
	void AnimationCrawling::RegisterEvents() {

		InputManager::RegisterInputEvent("LightSwipeLeft", LightSwipeLeftEvent);
		InputManager::RegisterInputEvent("LightSwipeRight", LightSwipeRightEvent);
		InputManager::RegisterInputEvent("HeavySwipeLeft", HeavySwipeLeftEvent);
		InputManager::RegisterInputEvent("HeavySwipeRight", HeavySwipeRightEvent);

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

		AnimationManager::RegisterEvent("GTS_Crawl_Swipe_On_R", "Crawl", GTS_Crawl_Swipe_On_R);
		AnimationManager::RegisterEvent("GTS_Crawl_Swipe_On_L", "Crawl", GTS_Crawl_Swipe_On_L);
		AnimationManager::RegisterEvent("GTS_Crawl_Swipe_Off_R", "Crawl", GTS_Crawl_Swipe_Off_R);
		AnimationManager::RegisterEvent("GTS_Crawl_Swipe_Off_L", "Crawl", GTS_Crawl_Swipe_Off_L);
		AnimationManager::RegisterEvent("GTS_Crawl_Swipe_Power_On_R", "Crawl", GTS_Crawl_Swipe_Power_On_R);
		AnimationManager::RegisterEvent("GTS_Crawl_Swipe_Power_On_L", "Crawl", GTS_Crawl_Swipe_Power_On_L);
		AnimationManager::RegisterEvent("GTS_Crawl_Swipe_Power_Off_R", "Crawl", GTS_Crawl_Swipe_Power_Off_R);
		AnimationManager::RegisterEvent("GTS_Crawl_Swipe_Power_Off_L", "Crawl", GTS_Crawl_Swipe_Power_Off_L);
		AnimationManager::RegisterEvent("GTS_Crawl_Swipe_ArmSfx_Start", "Crawl", GTS_Crawl_Swipe_ArmSfx_Start);
		AnimationManager::RegisterEvent("GTS_Crawl_Swipe_ArmSfx_End", "Crawl", GTS_Crawl_Swipe_ArmSfx_End);
	}

	void AnimationCrawling::RegisterTriggers() {
		AnimationManager::RegisterTrigger("SwipeLight_Left", "Crawl", "GTSBeh_SwipeLight_L");
		AnimationManager::RegisterTrigger("SwipeLight_Right", "Crawl", "GTSBeh_SwipeLight_R");
		AnimationManager::RegisterTrigger("SwipeHeavy_Right", "Crawl", "GTSBeh_SwipeHeavy_R");
		AnimationManager::RegisterTrigger("SwipeHeavy_Left", "Crawl", "GTSBeh_SwipeHeavy_L");
	}
}