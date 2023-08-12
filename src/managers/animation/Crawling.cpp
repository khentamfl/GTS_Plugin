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
		EnableHandTracking(&data.giant, CrawlEvent::RightHand, true);
	}
	void GTSCrawl_Slam_Raise_Arm_L(AnimationEventData& data) {	
		EnableHandTracking(&data.giant, CrawlEvent::LeftHand, true);
	}
	void GTSCrawl_Slam_Lower_Arm_R(AnimationEventData& data) {
	}
	void GTSCrawl_Slam_Lower_Arm_L(AnimationEventData& data) {
	}

	void GTSCrawl_Slam_Impact_R(AnimationEventData& data) {
		auto giant = &data.giant;
		float scale = get_visual_scale(giant);
		DoCrawlingFunctions(giant, scale, 1.1, 1.9, CrawlEvent::RightHand, "RightHandRumble", 22, 17);
	}

	void GTSCrawl_Slam_Impact_L(AnimationEventData& data) {
		auto giant = &data.giant;
		float scale = get_visual_scale(giant);
		DoCrawlingFunctions(giant, scale, 1.1, 1.9, CrawlEvent::LeftHand, "LeftHandRumble", 22, 17);
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

namespace Gts
{
	void AnimationCrawling::RegisterEvents() {
		AnimationManager::RegisterEvent("GTSCrawl_KneeImpact_L", "Crawl", GTSCrawl_KneeImpact_L);
		AnimationManager::RegisterEvent("GTSCrawl_KneeImpact_R", "Crawl", GTSCrawl_KneeImpact_R);
		AnimationManager::RegisterEvent("GTSCrawl_HandImpact_L", "Crawl", GTSCrawl_HandImpact_L);
		AnimationManager::RegisterEvent("GTSCrawl_HandImpact_R", "Crawl", GTSCrawl_HandImpact_R);
		AnimationManager::RegisterEvent("GTSCrawl_Slam_Raise_Arm_R", "Crawl", GTSCrawl_Slam_Raise_Arm_R);
		AnimationManager::RegisterEvent("GTSCrawl_Slam_Raise_Arm_L", "Crawl", GTSCrawl_Slam_Raise_Arm_L);
		AnimationManager::RegisterEvent("GTSCrawl_Slam_Lower_Arm_R", "Crawl", GTSCrawl_Slam_Lower_Arm_R);
		AnimationManager::RegisterEvent("GTSCrawl_Slam_Lower_Arm_L", "Crawl", GTSCrawl_Slam_Lower_Arm_L);
		AnimationManager::RegisterEvent("GTSCrawl_Slam_Impact_R", "Crawl", GTSCrawl_Slam_Impact_R);
		AnimationManager::RegisterEvent("GTSCrawl_Slam_Impact_L", "Crawl", GTSCrawl_Slam_Impact_L);
		
	}

	void AnimationCrawling::RegisterTriggers() {
	}
}