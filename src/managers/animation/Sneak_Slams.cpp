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
#include "ActionSettings.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "rays/raycast.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace { // WIP
	const std::string_view Rfinger = "NPC R Finger12 [RF12]";
	const std::string_view Lfinger = "NPC L Finger12 [LF12]";

	void EnableHandTracking(Actor* giant, CrawlEvent kind, bool enable) {
			if (kind == CrawlEvent::RightHand) {
				ManageCamera(giant, enable, 4.0);
			} else if (kind == CrawlEvent::LeftHand) {
				ManageCamera(giant, enable, 7.0);
			}
		}
	}

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

	void Finger_DamageAndShrink(Actor* giant, float radius, float damage, NiAVObject* node, float random, float bbmult, float crushmult, float ShrinkMult, DamageSource Cause) { // Apply crawl damage to each bone individually
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
								if (get_target_scale(otherActor) > 0.06 / GetSizeFromBoundingBox(otherActor)) {
									update_target_scale(otherActor, -0.0012 * ShrinkMult, SizeEffectType::kShrink);
								}
								CollisionDamage::GetSingleton().DoSizeDamage(giant, otherActor, damage, bbmult, crushmult, random, Cause);
							}
						}
					}
				}
			}
		}
	}

	void Finger_StartShrinkTask(Actor* giant, bool Right, float Radius, float Damage, float CrushMult) {
		auto gianthandle = giant->CreateRefHandle();

		std::string name = std::format("FingerShrink_{}", giant->formID);

		DamageSource source = DamageSource::RightFinger;
		std::string_view NodeLookup = Rfinger;
		if (!Right) {
			source = DamageSource::LeftFinger;
			NodeLookup = Lfinger;
		}

		TaskManager::Run(name, [=](auto& progressData) {
			if (!gianthandle) {
				return false;
			}

			auto giantref = gianthandle.get().get();

			if (IsFootGrinding(giantref)) {
				NiAVObject* node = find_node(giantref, NodeLookup);
				Finger_DamageAndShrink(giantref, Radius, Damage, node, 200, 0.05, CrushMult, 1.0, source);
				return true;
			}
			
			return false;
		});
	}

	void Finger_DoDamage(Actor* giant, bool Right, float Radius, float Damage, float CrushMult) {
		DamageSource source = DamageSource::RightFinger;
		std::string_view NodeLookup = Rfinger;
		if (!Right) {
			source = DamageSource::LeftFinger;
			NodeLookup = Lfinger;
		}

		NiAVObject* node = find_node(giant, NodeLookup);

		Finger_DamageAndShrink(giant, Radius, Damage, node, 50, 0.10, 2.5 * CrushMult, 25.0, source);
	}

	void StopStaminaDrain(Actor* giant) {
		DrainStamina(giant, "StaminaDrain_StrongSneakSlam", "DestructionBasics", false, 2.2);
		DrainStamina(giant, "StaminaDrain_FingerGrind", "DestructionBasics", false, 0.8);
		DrainStamina(giant, "StaminaDrain_SneakSlam", "DestructionBasics", false, 1.4);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// E V E N T S
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void GTSSneak_Slam_Raise_Arm_R(AnimationEventData& data) {
		EnableHandTracking(&data.giant, CrawlEvent::RightHand, true);
		DrainStamina(&data.giant, "StaminaDrain_SneakSlam", "DestructionBasics", true, 1.4);
	}
	void GTSSneak_Slam_Raise_Arm_L(AnimationEventData& data) {
		EnableHandTracking(&data.giant, CrawlEvent::LeftHand, true);
		DrainStamina(&data.giant, "StaminaDrain_SneakSlam", "DestructionBasics", true, 1.4);
	}

	void GTSSneak_SlamStrong_Raise_Arm_R(AnimationEventData& data) {
		DrainStamina(&data.giant, "StaminaDrain_StrongSneakSlam", "DestructionBasics", true, 2.2);
	}; 
	void GTSSneak_SlamStrong_Raise_Arm_L(AnimationEventData& data) {
		DrainStamina(&data.giant, "StaminaDrain_StrongSneakSlam", "DestructionBasics", true, 2.2);
	};

	void GTSSneak_Slam_Lower_Arm_R(AnimationEventData& data) {};
	void GTSSneak_Slam_Lower_Arm_L(AnimationEventData& data) {};

	void GTSSneak_SlamStrong_Lower_Arm_R(AnimationEventData& data) {}; 
	void GTSSneak_SlamStrong_Lower_Arm_L(AnimationEventData& data) {};
	
	void GTSSneak_Slam_Impact_R(AnimationEventData& data) {
		float scale = get_visual_scale(&data.giant);
		
		FingerGrindCheck(&data.giant, CrawlEvent::RightHand, true, Radius_Sneak_HandSlam);
		Finger_StartShrinkTask(&data.giant, true, Radius_Sneak_FingerGrind_DOT, Damage_Sneak_FingerGrind_DOT, 3.0);
		
	
		DoCrawlingFunctions(&data.giant, scale, 0.75, Damage_Sneak_HandSlam, CrawlEvent::RightHand, "RightHandRumble", 0.80, Radius_Sneak_HandSlam, 1.25, DamageSource::HandSlamRight);
	};
	void GTSSneak_Slam_Impact_L(AnimationEventData& data) {};
	
	void GTSSneak_SlamStrong_Impact_R(AnimationEventData& data) {
	};  
	void GTSSneak_SlamStrong_Impact_L(AnimationEventData& data) {};    

	void GTSSneak_Slam_Cam_Off_R(AnimationEventData& data) {
		EnableHandTracking(&data.giant, CrawlEvent::RightHand, false);
		StopStaminaDrain(&data.giant);
	};        
	void GTSSneak_Slam_Cam_Off_L(AnimationEventData& data) {
		EnableHandTracking(&data.giant, CrawlEvent::LeftHand, false);
		StopStaminaDrain(&data.giant);
	};

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void GTS_Sneak_FingerGrind_CameraOn(AnimationEventData& data) {
		EnableHandTracking(&data.giant, CrawlEvent::RightHand, true);
	};  
	void GTS_Sneak_FingerGrind_Impact(AnimationEventData& data) {
		Finger_DoDamage(&data.giant, true, Radius_Sneak_FingerGrind_Impact, Damage_Sneak_FingerGrind_Impact, 2.4);
		Finger_DoSounds(&data.giant, Rfinger, 1.0);
		Finger_ApplyVisuals(&data.giant, Rfinger, 3.0, 1.25);

		DrainStamina(&data.giant, "StaminaDrain_FingerGrind", "DestructionBasics", true, 0.8);
	};
	void GTS_Sneak_FingerGrind_Rotation(AnimationEventData& data) {
		Finger_DoDamage(&data.giant, true, Radius_Sneak_FingerGrind_DOT, Damage_Sneak_FingerGrind_DOT, 2.8);
		Finger_ApplyVisuals(&data.giant, Rfinger, 3.0, 0.75);
	};   
	void GTS_Sneak_FingerGrind_Finisher(AnimationEventData& data) {
		StopStaminaDrain(&data.giant);
		Finger_DoDamage(&data.giant, true, Radius_Sneak_FingerGrind_Finisher, Damage_Sneak_FingerGrind_Finisher, 1.8);
		Finger_DoSounds(&data.giant, Rfinger, 1.4);
		Finger_ApplyVisuals(&data.giant, Rfinger, 2.6, 1.5);
	};
	void GTS_Sneak_FingerGrind_CameraOff(AnimationEventData& data) {EnableHandTracking(&data.giant, CrawlEvent::RightHand, false);
}

namespace Gts {
    
    void Animation_SneakSlams::RegisterEvents() {
		AnimationManager::RegisterEvent("GTSSneak_Slam_Raise_Arm_R", "Sneak", GTSSneak_Slam_Raise_Arm_R);
		AnimationManager::RegisterEvent("GTSSneak_Slam_Raise_Arm_L", "Sneak", GTSSneak_Slam_Raise_Arm_L);

		AnimationManager::RegisterEvent("GTSSneak_SlamStrong_Raise_Arm_R", "Sneak", GTSSneak_SlamStrong_Raise_Arm_R);
		AnimationManager::RegisterEvent("GTSSneak_SlamStrong_Raise_Arm_L", "Sneak", GTSSneak_SlamStrong_Raise_Arm_L);

		AnimationManager::RegisterEvent("GTSSneak_Slam_Impact_R", "Sneak", GTSSneak_Slam_Impact_R);
		AnimationManager::RegisterEvent("GTSSneak_Slam_Impact_L", "Sneak", GTSSneak_Slam_Impact_L);

		AnimationManager::RegisterEvent("GTSSneak_SlamStrong_Impact_R", "Sneak", GTSSneak_SlamStrong_Impact_R);
		AnimationManager::RegisterEvent("GTSSneak_SlamStrong_Impact_L", "Sneak", GTSSneak_SlamStrong_Impact_L);

		AnimationManager::RegisterEvent("GTSSneak_Slam_Cam_Off_R", "Sneak", GTSSneak_Slam_Cam_Off_R);
		AnimationManager::RegisterEvent("GTSSneak_Slam_Cam_Off_L", "Sneak", GTSSneak_Slam_Cam_Off_L);

		AnimationManager::RegisterEvent("GTS_Sneak_FingerGrind_CameraOn", "Sneak", GTS_Sneak_FingerGrind_CameraOn);
		AnimationManager::RegisterEvent("GTS_Sneak_FingerGrind_Impact", "Sneak", GTS_Sneak_FingerGrind_Impact);

		AnimationManager::RegisterEvent("GTS_Sneak_FingerGrind_Rotation", "Sneak", GTS_Sneak_FingerGrind_Rotation);
		AnimationManager::RegisterEvent("GTS_Sneak_FingerGrind_Finisher", "Sneak", GTS_Sneak_FingerGrind_Finisher);

		AnimationManager::RegisterEvent("GTS_Sneak_FingerGrind_CameraOff", "Sneak", GTS_Sneak_FingerGrind_CameraOff);
    }
}