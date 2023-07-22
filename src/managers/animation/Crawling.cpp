#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/animation/AnimationManager.hpp"
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

    void SpawnCrawlParticle(Actor* actor, float scale, NiPoint3 position) {
        SpawnParticle(actor, 4.60, "GTS/Effects/Footstep.nif", NiMatrix3(), position, scale * 2.0, 7, nullptr);
    }

    std::string_view GetImpactNode(CrawlEvent kind) {
        if (kind == CrawlEvent::RightKnee) {
            return "NPC R Calf [RClf]";
        } else if (kind == CrawlEvent::LeftKnee) {
            return "NPC L Calf [LClf]";
        } else if (kind == CrawlEvent::RightHand) {
            return "NPC R Finger20 [RF20]";
        } else if (kind == CrawlEvent::LeftHand) {
            return "NPC L Finger20 [LF20]";
        } else {
            return "NPC L Finger20 [LF20]";
        }
    }

    void DoEverything(Actor* actor, float scale, float multiplier, CrawlEvent kind, std::string_view tag, float launch_dist, float damage_dist) {
        std::string_view name = GetImpactNode(kind);
        
        auto node = find_node(actor, name);
        if (!node) {
            return;
        }

        std::string rumbleName = std::format("{}{}", tag, actor->formID);
        Rumble::Once(rumbleName, actor, 0.80 * multiplier, 0.10, name); // Do Rumble

        LaunchActor::GetSingleton().LaunchCrawling(actor, launch_dist, 1.75 * multiplier, node, 0.75 * multiplier); // Launch actors
        AccurateDamage::GetSingleton().DoCrawlingDamage(actor, damage_dist, 45 * multiplier, node, 25, 0.05); // Do size-related damage
        FootStepManager::DirectImpact(actor, scale, node, FootEvent::Left); // Do impact sounds

        NiPoint3 node_location = node->world.translate;

        NiPoint3 ray_start = node_location + NiPoint3(0.0, 0.0, meter_to_unit(-0.05*scale)); // Shift up a little
        NiPoint3 ray_direction(0.0, 0.0, -1.0);
        bool success = false;
        float ray_length = meter_to_unit(std::max(1.05*scale, 1.05));
        NiPoint3 explosion_pos = CastRay(actor, ray_start, ray_direction, ray_length, success);

        if (!success) {
            explosion_pos = node_location;
            explosion_pos.z = actor->GetPosition().z;
        }
        if (actor->formID == 0x14 && Runtime::GetBool("PCAdditionalEffects")) {
            SpawnCrawlParticle(actor, scale * multiplier, explosion_pos);
        }
        if (actor->formID != 0x14 && Runtime::GetBool("NPCSizeEffects")) {
            SpawnCrawlParticle(actor, scale * multiplier, explosion_pos);
        }
    }

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  ////////////////////                        E V E N T S
  /////////////////////////////////////////////////////////////////////////////////////////////////////

    void GTSCrawl_KneeImpact_L(AnimationEventData& data) {
        auto giant = &data.giant;
        float scale = get_visual_scale(giant);
        DoEverything(giant, scale, 1.25, CrawlEvent::LeftKnee, "LeftKnee", 32, 18);
	}
	void GTSCrawl_KneeImpact_R(AnimationEventData& data) {
        auto giant = &data.giant;
        float scale = get_visual_scale(giant);
        DoEverything(giant, scale, 1.25, CrawlEvent::RightKnee, "RightKnee", 32, 18);
	}
	void GTSCrawl_HandImpact_L(AnimationEventData& data) {
        auto giant = &data.giant;
        float scale = get_visual_scale(giant);
        DoEverything(giant, scale, 1.0, CrawlEvent::LeftHand, "LeftHand", 26, 14);
	}
	void GTSCrawl_HandImpact_R(AnimationEventData& data) {
        auto giant = &data.giant;
        float scale = get_visual_scale(giant);
        DoEverything(giant, scale, 1.0, CrawlEvent::RightHand, "RightHand", 26, 14);
	}
     /////////////////////////////////////////////////////////////////////////////////////////////////////
  ////////////////////                        E V E N T S  E N D
  /////////////////////////////////////////////////////////////////////////////////////////////////////
}

namespace Gts
{
	void AnimationCrawling::RegisterEvents() {
		AnimationManager::RegisterEvent("GTSCrawl_KneeImpact_L", "Crawl", GTSCrawl_KneeImpact_L);
		AnimationManager::RegisterEvent("GTSCrawl_KneeImpact_R", "Crawl", GTSCrawl_KneeImpact_R);
		AnimationManager::RegisterEvent("GTSCrawl_HandImpact_L", "Crawl", GTSCrawl_HandImpact_L);
		AnimationManager::RegisterEvent("GTSCrawl_HandImpact_R", "Crawl", GTSCrawl_HandImpact_R);
	}

	void AnimationCrawling::RegisterTriggers() {
	}
}