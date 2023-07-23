#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/animation/Utils/CrawlUtils.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/damage/AccurateDamage.hpp"
#include "managers/damage/LaunchActor.hpp"
#include "managers/animation/Crawling.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/CrushManager.hpp"
#include "managers/footstep.hpp"
#include "utils/actorUtils.hpp"
#include "data/persistent.hpp"
#include "managers/Rumble.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "raycast.hpp"

namespace {
    void SpawnCrawlParticle(Actor* actor, float scale, NiPoint3 position) {
        SpawnParticle(actor, 4.60, "GTS/Effects/Footstep.nif", NiMatrix3(), position, scale * 1.8, 7, nullptr);
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
}

namespace Gts {

    void DoCrawlingSounds(Actor* actor, float scale, NiAVObject* node, FootEvent foot_kind) { // Call crawling sounds
		if (actor) {
			auto profiler = Profilers::Profile("CrawlSounds");
			auto player = PlayerCharacter::GetSingleton();
			if (actor->formID == 0x14 && HasSMT(actor)) {
				scale *= 1.85;
			}
			float sprint_factor = 1.0;
			bool LegacySounds = Persistent::GetSingleton().legacy_sounds; // Determine if we should play old pre 2.00 update sounds
			bool sprinting = false;
			if (scale > 1.2 && !actor->AsActorState()->IsSwimming()) {
				float start_l = 1.2;
				float start_xl = 11.99;
				float start_xlJumpLand= 1.99;
				float start_xxl = 20.0;
				scale *= 0.60;
				if (actor->formID == 0x14 && IsFirstPerson()) { // Footsteps are quieter when in first person
					scale *= 0.70;
				}

				if (actor->AsActorState()->IsWalking()) {
					scale *= 0.75; // Walking makes you sound quieter
				}

				if (Runtime::GetBool("EnableGiantSounds")) {
					if (!LegacySounds) {       // Play normal sounds
						FootStepManager::PlayNormalSounds(node, foot_kind, scale, sprint_factor, sprinting);
						return;
					} else if (LegacySounds) { // Play old sounds
						FootStepManager::PlayLegacySounds(node, foot_kind, scale, start_l, start_xl, start_xxl);
						return;
					}
				}
			}
		}
	}

    void DoCrawlingFunctions(Actor* actor, float scale, float multiplier, CrawlEvent kind, std::string_view tag, float launch_dist, float damage_dist) { // Call everything
        std::string_view name = GetImpactNode(kind);
        
        auto node = find_node(actor, name);
        if (!node) {
            return; // Make sure to return if node doesn't exist, no CTD in that case
        }

        std::string rumbleName = std::format("{}{}", tag, actor->formID);
        Rumble::Once(rumbleName, actor, 0.90 * multiplier, 0.10, name); // Do Rumble

        LaunchActor::GetSingleton().LaunchCrawling(actor, launch_dist, 0.25 * multiplier, node, 0.75 * multiplier); // Launch actors
		//                                                   radius           power       object       damage

        

        float minimal_scale = 2.0;

        if (actor->formID == 0x14) {
			if (HasSMT(actor)) {
				multiplier *= 1.8;
				minimal_scale = 1.0;
				scale += 0.75;
			}
		}

		DoCrawlingDamage(actor, damage_dist, 70 * multiplier, node, 20, 0.05); // Do size-related damage
        DoCrawlingSounds(actor, scale, node, FootEvent::Left);                 // Do impact sounds

        if (scale >= minimal_scale && !actor->AsActorState()->IsSwimming()) {
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
    }


    void DoCrawlingDamage(Actor* giant, float radius, float damage, NiAVObject* node, float random, float bbmult) { // Apply crawl damage to each bone individually
        auto profiler = Profilers::Profile("Other: CrawlDamage");
		if (!node) {
			return;
		} if (!giant) {
			return;
		}
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
							float force = 1.0 - distance / maxDistance;
                            float aveForce = std::clamp(force, 0.00f, 0.70f);
							AccurateDamage::GetSingleton().ApplySizeEffect(giant, otherActor, aveForce * damage, random, bbmult);
                        }
					}
				}
			}
		}
	}

void ApplyAllCrawlingDamage(Actor* giant, float damage, int random, float bonedamage) { // Applies damage to all 4 crawl bones at once
        auto LC = find_node(giant, "NPC L Calf [LClf]");
        auto RC = find_node(giant, "NPC R Calf [RClf]");
        auto LH = find_node(giant, "NPC L Finger20 [LF20]");
        auto RH = find_node(giant, "NPC R Finger20 [RF20]");
		if (!LC) {
			return;
		} if (!RC) {
			return;
		} if (!LH) {
			return;
		} if (!RH) {
			return;
		} // CTD protection

		int grabbed;
		giant->GetGraphVariableInt("GTS_GrabbedTiny", grabbed);
		

		DoCrawlingDamage(giant, 10, damage, LC, random, bonedamage); 		// Call Left Calf
		DoCrawlingDamage(giant, 10, damage, RC, random, bonedamage);        // Call Right Calf

		if (grabbed <= 0) { // Only do if we don't have someone in our left hand
			DoCrawlingDamage(giant, 7, damage * 0.8, LH, random, bonedamage);   // Call Left Hand
		}
		
		DoCrawlingDamage(giant, 7, damage * 0.8, RH, random, bonedamage);   // Call Right Calf
	}
}