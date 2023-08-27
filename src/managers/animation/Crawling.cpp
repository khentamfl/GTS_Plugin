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

	float GetWasteMult(Actor* giant) {
		float WasteMult = 1.0;
		if (Runtime::HasPerk(giant, "DestructionBasics")) {
			WasteMult *= 0.65;
		}
		if (Runtime::HasPerkTeam(giant, "SkilledGTS")) {
			WasteMult -= GetGtsSkillLevel() * 0.0035;
		}
		return WasteMult;
	}

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

	void DoDamageAtPoint_Cooldown(Actor* giant, float radius, float damage, NiAVObject* node, float random, float bbmult, float crushmult, float pushpower) { // Apply crawl damage to each bone individually
        auto profiler = Profilers::Profile("Other: CrawlDamage");
		if (!node) {
			return;
		} if (!giant) {
			return;
		}

		auto& sizemanager = SizeManager::GetSingleton();
		float giantScale = get_visual_scale(giant);

		float SCALE_RATIO = 1.25;
		if (HasSMT(giant)) {
			SCALE_RATIO = 1.10;
			giantScale *= 2.0;
		}

		NiPoint3 NodePosition = node->world.translate;

		float maxDistance = radius * giantScale;
		// Make a list of points to check
		std::vector<NiPoint3> points = {
			NiPoint3(0.0, 0.0, 0.0), // The standard position
		};
		std::vector<NiPoint3> CrawlPoints = {};

		for (NiPoint3 point: points) {
			CrawlPoints.push_back(NodePosition);
		}
		if (Runtime::GetBool("EnableDebugOverlay") && (giant->formID == 0x14 || giant->IsPlayerTeammate() || Runtime::InFaction(giant, "FollowerFaction"))) {
			for (auto point: CrawlPoints) {
				DebugAPI::DrawSphere(glm::vec3(point.x, point.y, point.z), maxDistance);
			}
		}

		NiPoint3 giantLocation = giant->GetPosition();

		for (auto otherActor: find_actors()) {
			if (otherActor != giant) { 
				float tinyScale = get_visual_scale(otherActor);
				if (giantScale / tinyScale > SCALE_RATIO) {
					NiPoint3 actorLocation = otherActor->GetPosition();
					for (auto point: CrawlPoints) {
						float distance = (point - actorLocation).Length();
						if (distance <= maxDistance) {
							bool allow = sizemanager.IsHandDamaging(otherActor);
							if (!allow) {
								float force = 1.0 - distance / maxDistance;
								float aveForce = std::clamp(force, 0.00f, 0.70f);
								float pushForce = std::clamp(force, 0.01f, 0.10f);
								AccurateDamage::GetSingleton().ApplySizeEffect(giant, otherActor, aveForce * damage, random, bbmult, crushmult);
								PushTowards(giant, otherActor, node, pushForce * pushpower, true);
								sizemanager.GetDamageData(otherActor).lastHandDamageTime = Time::WorldTimeElapsed();
							}
                        }
					}
				}
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
				DoDamageAtPoint_Cooldown(giant, 17, 80.0 * power, Uarm, 10, 0.30, crush, pushpower, DamageSource::HandSwipe);
			}
			if (Arm) {
				DoDamageAtPoint_Cooldown(giant, 19, 80.0 * power, Arm, 10, 0.30, crush, pushpower, DamageSource::HandSwipe);
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
				DoDamageAtPoint_Cooldown(giant, 17, 80.0 * power, Uarm, 10, 0.30, crush, pushpower, DamageSource::HandSwipe);
			}
			if (Arm) {
				DoDamageAtPoint_Cooldown(giant, 19, 80.0 * power, Arm, 10, 0.30, crush, pushpower, DamageSource::HandSwipe);
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

	void GTS_Crawl_Swipe_On_R(AnimationEventData& data) {
		TriggerHandCollision_Right(&data.giant, 1.0, 1.6, 0.75);
	}
	void GTS_Crawl_Swipe_On_L(AnimationEventData& data) {
		TriggerHandCollision_Left(&data.giant, 1.0, 1.6, 0.75);
	}
	void GTS_Crawl_Swipe_Off_R(AnimationEventData& data) {
		DisableHandCollisions(&data.giant);
	}
	void GTS_Crawl_Swipe_Off_L(AnimationEventData& data) {
		DisableHandCollisions(&data.giant);
	}

	void GTS_Crawl_Swipe_Power_On_R(AnimationEventData& data) {
		TriggerHandCollision_Right(&data.giant, 2.3, 1.3, 2.2);
	}
	void GTS_Crawl_Swipe_Power_On_L(AnimationEventData& data) {
		TriggerHandCollision_Left(&data.giant, 2.3, 1.3, 2.2);
	}
	void GTS_Crawl_Swipe_Power_Off_R(AnimationEventData& data) {
		DisableHandCollisions(&data.giant);
	}
	void GTS_Crawl_Swipe_Power_Off_L(AnimationEventData& data) {
		DisableHandCollisions(&data.giant);
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
	}

	void AnimationCrawling::RegisterTriggers() {
		AnimationManager::RegisterTrigger("SwipeLight_Left", "Crawl", "GTSBeh_SwipeLight_L");
		AnimationManager::RegisterTrigger("SwipeLight_Right", "Crawl", "GTSBeh_SwipeLight_R");
		AnimationManager::RegisterTrigger("SwipeHeavy_Right", "Crawl", "GTSBeh_SwipeHeavy_R");
		AnimationManager::RegisterTrigger("SwipeHeavy_Left", "Crawl", "GTSBeh_SwipeHeavy_L");
	}
}