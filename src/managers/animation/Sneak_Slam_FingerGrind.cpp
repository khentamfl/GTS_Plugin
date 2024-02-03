#include "managers/animation/Sneak_Slam_FingerGrind.hpp"
#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/animation/Utils/CrawlUtils.hpp"
#include "managers/damage/CollisionDamage.hpp"
#include "managers/damage/LaunchActor.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/CrushManager.hpp"
#include "managers/InputManager.hpp"
#include "managers/footstep.hpp"
#include "utils/actorUtils.hpp"
#include "managers/Rumble.hpp"
#include "ActionSettings.hpp"
#include "data/runtime.hpp"
#include "rays/raycast.hpp"
#include "scale/scale.hpp"


using namespace std;
using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace {
    const std::string_view Rfinger = "NPC R Finger12 [RF12]";
	const std::string_view Lfinger = "NPC L Finger12 [LF12]";

    void Finger_DoSounds(Actor* giant, std::string_view node_name, float mult) {
		NiAVObject* node = find_node(giant, node_name);
		if (node) {
			float scale = get_visual_scale(giant) * 0.6;
			DoCrawlingSounds(giant, scale * mult, node, FootEvent::Left);
		}
	}

	void Finger_ApplyVisuals(Actor* giant, std::string_view node_name, float threshold, float multiplier) {
		NiAVObject* node = find_node(giant, node_name);
		if (node) {
			float min_scale = 3.5 * threshold;
			float scale = get_visual_scale(giant);
			if (HasSMT(giant)) {
				scale += 2.6;
				multiplier *= 0.33;
			}
			if (scale >= threshold && !giant->AsActorState()->IsSwimming()) {
				NiPoint3 node_location = node->world.translate;

				NiPoint3 ray_start = node_location + NiPoint3(0.0, 0.0, meter_to_unit(-0.05*scale)); // Shift up a little
				NiPoint3 ray_direction(0.0, 0.0, -1.0);
				bool success = false;
				float ray_length = meter_to_unit(std::max(1.05*scale, 1.05));
				NiPoint3 explosion_pos = CastRay(giant, ray_start, ray_direction, ray_length, success);

				if (!success) {
					explosion_pos = node_location;
					explosion_pos.z = giant->GetPosition().z;
				}
				if (giant->formID == 0x14 && Runtime::GetBool("PCAdditionalEffects")) {
					SpawnParticle(giant, 4.60, "GTS/Effects/Footstep.nif", NiMatrix3(), explosion_pos, (scale * multiplier) * 1.8, 7, nullptr);
				}
				if (giant->formID != 0x14 && Runtime::GetBool("NPCSizeEffects")) {
					SpawnParticle(giant, 4.60, "GTS/Effects/Footstep.nif", NiMatrix3(), explosion_pos, (scale * multiplier) * 1.8, 7, nullptr);
				}
			}
		}
	}
    void Finger_DoDamage(Actor* giant, bool Right, float Radius, float Damage, float CrushMult, float ShrinkMult) {
		DamageSource source = DamageSource::RightFinger;
		std::string_view NodeLookup = Rfinger;
		if (!Right) {
			source = DamageSource::LeftFinger;
			NodeLookup = Lfinger;
		}

		NiAVObject* node = find_node(giant, NodeLookup);

		Finger_DamageAndShrink(giant, Radius, Damage, node, 50, 0.10, CrushMult, -0.028 * ShrinkMult, source);
	}

    ////////////////////////////////////////////////////////////////////
    /////// Events
    ///////////////////////////////////////////////////////////////////

    void GTS_Sneak_FingerGrind_CameraOn_R(AnimationEventData& data) {
		ManageCamera(&data.giant, true, CameraTracking::Finger_Right);
	};  

    void GTS_Sneak_FingerGrind_CameraOn_L(AnimationEventData& data) {
		ManageCamera(&data.giant, true, CameraTracking::Finger_Left);
	};  

	void GTS_Sneak_FingerGrind_Impact_R(AnimationEventData& data) {
		Finger_DoDamage(&data.giant, true, Radius_Sneak_FingerGrind_Impact, Damage_Sneak_FingerGrind_Impact, 2.8, 1.2);
		Finger_DoSounds(&data.giant, Rfinger, 1.0);
		Finger_ApplyVisuals(&data.giant, Rfinger, 2.6, 1.0);

		DrainStamina(&data.giant, "StaminaDrain_FingerGrind", "DestructionBasics", true, 0.8);
	};
    void GTS_Sneak_FingerGrind_Impact_L(AnimationEventData& data) {
		Finger_DoDamage(&data.giant, false, Radius_Sneak_FingerGrind_Impact, Damage_Sneak_FingerGrind_Impact, 2.8, 1.2);
		Finger_DoSounds(&data.giant, Lfinger, 1.0);
		Finger_ApplyVisuals(&data.giant, Lfinger, 2.6, 1.0);

		DrainStamina(&data.giant, "StaminaDrain_FingerGrind", "DestructionBasics", true, 0.8);
	};

	void GTS_Sneak_FingerGrind_Rotation_R(AnimationEventData& data) {
		Finger_DoDamage(&data.giant, true, Radius_Sneak_FingerGrind_DOT, Damage_Sneak_FingerGrind_DOT, 3.2, 0.8);
		Finger_ApplyVisuals(&data.giant, Rfinger, 2.6, 0.85);
	};   
    void GTS_Sneak_FingerGrind_Rotation_L(AnimationEventData& data) {
		Finger_DoDamage(&data.giant, false, Radius_Sneak_FingerGrind_DOT, Damage_Sneak_FingerGrind_DOT, 3.2, 0.8);
		Finger_ApplyVisuals(&data.giant, Lfinger, 2.6, 0.85);
	};   

	void GTS_Sneak_FingerGrind_Finisher_R(AnimationEventData& data) {
		Finger_DoDamage(&data.giant, true, Radius_Sneak_FingerGrind_Finisher, Damage_Sneak_FingerGrind_Finisher, 2.4, 3.0);
        Finger_ApplyVisuals(&data.giant, Rfinger, 2.6, 1.2);
		Finger_DoSounds(&data.giant, Rfinger, 1.4);
        StopStaminaDrain(&data.giant);	
	};
    void GTS_Sneak_FingerGrind_Finisher_L(AnimationEventData& data) {
		Finger_DoDamage(&data.giant, false, Radius_Sneak_FingerGrind_Finisher, Damage_Sneak_FingerGrind_Finisher, 2.4, 3.0);
        Finger_ApplyVisuals(&data.giant, Lfinger, 2.6, 1.2);
		Finger_DoSounds(&data.giant, Lfinger, 1.4);
        StopStaminaDrain(&data.giant);
		
	};

	void GTS_Sneak_FingerGrind_CameraOff_R(AnimationEventData& data) {TrackMatchingHand(&data.giant, CrawlEvent::RightHand, false);}
    void GTS_Sneak_FingerGrind_CameraOff_L(AnimationEventData& data) {TrackMatchingHand(&data.giant, CrawlEvent::LeftHand, false);}

}

namespace Gts {
    void Animation_SneakSlam_FingerGrind::RegisterEvents() {
        AnimationManager::RegisterEvent("GTS_Sneak_FingerGrind_CameraOn_R", "Sneak", GTS_Sneak_FingerGrind_CameraOn_R);
        AnimationManager::RegisterEvent("GTS_Sneak_FingerGrind_CameraOn_L", "Sneak", GTS_Sneak_FingerGrind_CameraOn_L);

		AnimationManager::RegisterEvent("GTS_Sneak_FingerGrind_Impact_R", "Sneak", GTS_Sneak_FingerGrind_Impact_R);
        AnimationManager::RegisterEvent("GTS_Sneak_FingerGrind_Impact_L", "Sneak", GTS_Sneak_FingerGrind_Impact_L);

		AnimationManager::RegisterEvent("GTS_Sneak_FingerGrind_Rotation_R", "Sneak", GTS_Sneak_FingerGrind_Rotation_R);
        AnimationManager::RegisterEvent("GTS_Sneak_FingerGrind_Rotation_L", "Sneak", GTS_Sneak_FingerGrind_Rotation_L);

		AnimationManager::RegisterEvent("GTS_Sneak_FingerGrind_Finisher_R", "Sneak", GTS_Sneak_FingerGrind_Finisher_R);
        AnimationManager::RegisterEvent("GTS_Sneak_FingerGrind_Finisher_L", "Sneak", GTS_Sneak_FingerGrind_Finisher_L);

		AnimationManager::RegisterEvent("GTS_Sneak_FingerGrind_CameraOff_R", "Sneak", GTS_Sneak_FingerGrind_CameraOff_R);
        AnimationManager::RegisterEvent("GTS_Sneak_FingerGrind_CameraOff_L", "Sneak", GTS_Sneak_FingerGrind_CameraOff_L);
    }

    void Finger_DamageAndShrink(Actor* giant, float radius, float damage, NiAVObject* node, float random, float bbmult, float crushmult, float Shrink, DamageSource Cause) { // Apply crawl damage to each bone individually
		auto profiler = Profilers::Profile("Other: CrawlDamage");
		if (!node) {
			return;
		}
		if (!giant) {
			return;
		}
		float giantScale = get_visual_scale(giant);

		float SCALE_RATIO = 1.25;
		if (HasSMT(giant)) {
			SCALE_RATIO = 0.9;
			giantScale *= 1.3;
		}
		static Timer SmileTimer = Timer(3.60);
		NiPoint3 NodePosition = node->world.translate;

		float maxDistance = radius * giantScale;
		float CheckDistance = 220 * giantScale;
		// Make a list of points to check
		std::vector<NiPoint3> points = {
			NiPoint3(0.0, 0.0, 0.0), // The standard position
		};
		std::vector<NiPoint3> CrawlPoints = {};

		for (NiPoint3 point: points) {
			CrawlPoints.push_back(NodePosition);
		}
		if (IsDebugEnabled() && (giant->formID == 0x14 || IsTeammate(giant) || EffectsForEveryone(giant))) {
			for (auto point: CrawlPoints) {
				DebugAPI::DrawSphere(glm::vec3(point.x, point.y, point.z), maxDistance, 400.0);
			}
		}

		Utils_UpdateHighHeelBlend(giant, false);
		NiPoint3 giantLocation = giant->GetPosition();
		

		for (auto otherActor: find_actors()) {
			if (otherActor != giant) {
				float tinyScale = get_visual_scale(otherActor);
				if (giantScale / tinyScale > SCALE_RATIO) {
					NiPoint3 actorLocation = otherActor->GetPosition();
					for (auto point: CrawlPoints) {
						if ((actorLocation-giantLocation).Length() <= CheckDistance) {

							int nodeCollisions = 0;
							float force = 0.0;

							auto model = otherActor->GetCurrent3D();

							if (model) {
								VisitNodes(model, [&nodeCollisions, &force, NodePosition, maxDistance](NiAVObject& a_obj) {
									float distance = (NodePosition - a_obj.world.translate).Length();
									if (distance < maxDistance) {
										nodeCollisions += 1;
										force = 1.0 - distance / maxDistance;
										return false;
									}
									return true;
								});
							}
							if (nodeCollisions > 0) {
								if (get_target_scale(otherActor) > 0.08 / GetSizeFromBoundingBox(otherActor)) {
									update_target_scale(otherActor, Shrink, SizeEffectType::kShrink);
								} else {
									set_target_scale(otherActor, 0.08 / GetSizeFromBoundingBox(otherActor));
								}
								Laugh_Chance(giant, otherActor, 1.0, "FingerGrind"); 
								Utils_PushCheck(giant, otherActor, 1.0);
								CollisionDamage::GetSingleton().DoSizeDamage(giant, otherActor, damage, bbmult, crushmult, random, Cause);
							}
						}
					}
				}
			}
		}
	}

	void Laugh_Chance(Actor* giant, Actor* otherActor, float multiply, std::string_view name) {
		if (SmileTimer.ShouldRunFrame()) {
			int rng = rand() % 2 + 1;
			if (rng <= 1.0) {
				float duration = 1.5 + ((rand() % 100) * 0.01);
				duration * multiply;

				if (!otherActor->IsDead()) {
					PlayLaughSound(giant, 1.0, 1);
					Task_FacialEmotionTask_Smile(giant, duration, name);
				}
			}
		}
	}

    void TrackMatchingHand(Actor* giant, CrawlEvent kind, bool enable) {
        if (kind == CrawlEvent::RightHand) {
            ManageCamera(giant, enable, CameraTracking::Hand_Right);
        } else if (kind == CrawlEvent::LeftHand) {
            ManageCamera(giant, enable, CameraTracking::Hand_Left);
        }
    }

    void StopStaminaDrain(Actor* giant) {
		DrainStamina(giant, "StaminaDrain_StrongSneakSlam", "DestructionBasics", false, 2.2);
		DrainStamina(giant, "StaminaDrain_FingerGrind", "DestructionBasics", false, 0.8);
		DrainStamina(giant, "StaminaDrain_SneakSlam", "DestructionBasics", false, 1.4);
	}
}