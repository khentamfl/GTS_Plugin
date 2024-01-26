#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/animation/Utils/CrawlUtils.hpp"
#include "managers/damage/CollisionDamage.hpp"
#include "managers/damage/LaunchActor.hpp"
#include "managers/animation/Sneak_Slams.hpp"
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

namespace { // WIP

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

	void GTSCrawl_HandImpact_L(AnimationEventData& data) {
		auto giant = &data.giant;
		float scale = get_visual_scale(giant);
		if (IsTransferingTiny(giant)) {
			return; // Prevent effects from left hand
		}
		DoCrawlingFunctions(giant, scale, 1.10, 1.0, CrawlEvent::LeftHand, "LeftHand", 0.55, 14, 1.25, DamageSource::HandCrawlLeft);
	}
	void GTSCrawl_HandImpact_R(AnimationEventData& data) {
		auto giant = &data.giant;
		float scale = get_visual_scale(giant);
		DoCrawlingFunctions(giant, scale, 1.10, 1.0, CrawlEvent::RightHand, "RightHand", 0.55, 14, 1.0, DamageSource::HandCrawlRight);
		//                                                                               ^    ^ --- Size Damage Radius
		//                                                                             Launch
		//                                                                             Radius
	}

	void GTSCrawl_Slam_Raise_Arm_R(AnimationEventData& data) {
		DrainStamina(&data.giant, "StaminaDrain_CrawlStomp", "DestructionBasics", true, 1.4);
		EnableHandTracking(&data.giant, CrawlEvent::RightHand, true);
	}

	void GTSCrawl_Slam_Raise_Arm_L(AnimationEventData& data) {
		DrainStamina(&data.giant, "StaminaDrain_CrawlStomp", "DestructionBasics", true, 1.4);
		EnableHandTracking(&data.giant, CrawlEvent::LeftHand, true);
	}
	void GTSCrawl_SlamStrong_Raise_Arm_R(AnimationEventData& data) {
		DrainStamina(&data.giant, "StaminaDrain_CrawlStompStrong", "DestructionBasics", true, 2.3);
		EnableHandTracking(&data.giant, CrawlEvent::RightHand, true);
	}

	void GTSCrawl_SlamStrong_Raise_Arm_L(AnimationEventData& data) {
		DrainStamina(&data.giant, "StaminaDrain_CrawlStompStrong", "DestructionBasics", true, 2.3);
		EnableHandTracking(&data.giant, CrawlEvent::LeftHand, true);
	}

	void GTSCrawl_Slam_Lower_Arm_R(AnimationEventData& data) {
	}
	void GTSCrawl_SlamStrong_Lower_Arm_R(AnimationEventData& data) {
	}
	void GTSCrawl_Slam_Lower_Arm_L(AnimationEventData& data) {
	}
	void GTSCrawl_SlamStrong_Lower_Arm_L(AnimationEventData& data) {
	}

	void GTSCrawl_Slam_Impact_R(AnimationEventData& data) {
		auto giant = &data.giant;
		float scale = get_visual_scale(giant);
		DoCrawlingFunctions(giant, scale, 1.1, 2.1, CrawlEvent::RightHand, "RightHandRumble", 0.60, 17, 1.15, DamageSource::HandSlamRight);
		DrainStamina(&data.giant, "StaminaDrain_CrawlStomp", "DestructionBasics", false, 1.4);
		DrainStamina(&data.giant, "StaminaDrain_CrawlStompStrong", "DestructionBasics", false, 2.3);
	}
	void GTSCrawl_Slam_Impact_L(AnimationEventData& data) {
		auto giant = &data.giant;
		float scale = get_visual_scale(giant);
		DoCrawlingFunctions(giant, scale, 1.1, 2.1, CrawlEvent::LeftHand, "LeftHandRumble", 0.60, 17, 1.15, DamageSource::HandSlamLeft);
		DrainStamina(&data.giant, "StaminaDrain_CrawlStomp", "DestructionBasics", false, 1.4);
		DrainStamina(&data.giant, "StaminaDrain_CrawlStompStrong", "DestructionBasics", false, 2.3);
	}


	void GTSCrawl_SlamStrong_Impact_R(AnimationEventData& data) {
		auto giant = &data.giant;
		float scale = get_visual_scale(giant);
		DoCrawlingFunctions(giant, scale, 1.25, 5.2, CrawlEvent::RightHand, "RightHandRumble", 0.80, 18, 1.0, DamageSource::HandSlamRight);
		DrainStamina(&data.giant, "StaminaDrain_CrawlStomp", "DestructionBasics", false, 1.4);
		DrainStamina(&data.giant, "StaminaDrain_CrawlStompStrong", "DestructionBasics", false, 2.3);
	}
	void GTSCrawl_SlamStrong_Impact_L(AnimationEventData& data) {
		auto giant = &data.giant;
		float scale = get_visual_scale(giant);
		DoCrawlingFunctions(giant, scale, 1.25, 5.2, CrawlEvent::LeftHand, "RightHandRumble", 0.80, 18, 1.0, DamageSource::HandSlamLeft);
		DrainStamina(&data.giant, "StaminaDrain_CrawlStomp", "DestructionBasics", false, 1.4);
		DrainStamina(&data.giant, "StaminaDrain_CrawlStompStrong", "DestructionBasics", false, 2.3);
	}

	void GTSCrawl_Slam_Cam_Off_R(AnimationEventData& data) {
		auto giant = &data.giant;
		EnableHandTracking(&data.giant, CrawlEvent::RightHand, false);
	}
	void GTSCrawl_Slam_Cam_Off_L(AnimationEventData& data) {
		auto giant = &data.giant;
		EnableHandTracking(&data.giant, CrawlEvent::LeftHand, false);
	}
}

namespace Gts {
    
    void Animation_SneakSlams::RegisterEvents() {
		//AnimationManager::RegisterEvent("GTSstompimpactR", "Stomp", GTSstompimpactR);
		//AnimationManager::RegisterEvent("GTSstompimpactL", "Stomp", GTSstompimpactL);
    }
}