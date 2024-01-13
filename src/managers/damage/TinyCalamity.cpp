#include "managers/damage/tinyCalamity.hpp"
#include "managers/CrushManager.hpp"
#include "magic/effects/common.hpp"
#include "managers/Attributes.hpp"
#include "utils/actorUtils.hpp"
#include "data/persistent.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "UI/DebugAPI.hpp"

#include "profiler.hpp"

#include "node.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace {
    void RefreshDuration(Actor* giant) {
        if (Runtime::HasPerk(giant, "NoSpeedLoss")) {
            AttributeManager::GetSingleton().OverrideSMTBonus(0.65); // Reduce speed after crush
        } else {
            AttributeManager::GetSingleton().OverrideSMTBonus(0.35); // Reduce more speed after crush
        }
    }

    bool Collision_AllowTinyCalamityCrush(Actor* giant, Actor* tiny) {
        float giantHp = GetAV(giant, ActorValue::kHealth);
		float tinyHp = GetAV(tiny, ActorValue::kHealth);

        float Multiplier = get_visual_scale(giant) + 2.0/get_visual_scale(tiny);

        if (giantHp >= ((tinyHp / Multiplier) * 1.25)) {
            return true;
        } else {
            return false;
        }
    }
}

namespace Gts {
    void TinyCalamity_SeekActors(Actor* giant) {
        if (giant->formID == 0x14 && giant->AsActorState()->IsSprinting() && HasSMT(giant)) {
            auto node = find_node(giant, "NPC Pelvis [Pelv]");
            if (!node) {
                return;
            }
            NiPoint3 NodePosition = node->world.translate;

            float giantScale = get_visual_scale(giant);

            const float BASE_DISTANCE = 48.0;
            float CheckDistance = BASE_DISTANCE*giantScale;

            if (IsDebugEnabled() && (giant->formID == 0x14 || IsTeammate(giant))) {
                DebugAPI::DrawSphere(glm::vec3(NodePosition.x, NodePosition.y, NodePosition.z), CheckDistance, 100, {0.0, 1.0, 1.0, 1.0});
            }

            NiPoint3 giantLocation = giant->GetPosition();
            for (auto otherActor: find_actors()) {
                if (otherActor != giant) {
                    NiPoint3 actorLocation = otherActor->GetPosition();
                    if ((actorLocation - giantLocation).Length() < BASE_DISTANCE*giantScale*3) {
                        int nodeCollisions = 0;
                        float force = 0.0;

                        auto model = otherActor->GetCurrent3D();

                        if (model) {
                            VisitNodes(model, [&nodeCollisions, &force, NodePosition, CheckDistance](NiAVObject& a_obj) {
                                float distance = (NodePosition - a_obj.world.translate).Length();
                                if (distance < CheckDistance) {
                                    nodeCollisions += 1;
                                    force = 1.0 - distance / CheckDistance;
                                    return false;
                                }
                                return true;
                            });
                        }
                        if (nodeCollisions > 0) {
                            TinyCalamity_CrushCheck(giant, otherActor);
                        }
                    }
                }
            }
        }
    }

    void TinyCalamity_CrushCheck(Actor* giant, Actor* tiny) {
		auto profiler = Profilers::Profile("tinyCalamity: CrushCheck");
		if (giant == tiny) {
			return;
		}
		auto& persistent = Persistent::GetSingleton();
		if (persistent.GetData(giant)) {
			if (persistent.GetData(giant)->smt_run_speed >= 1.0) {
                float giantHp = GetAV(giant, ActorValue::kHealth);

                if (CrushManager::AlreadyCrushed(tiny)) {
                    return;
                }

				if (giantHp <= 0) {
					return; // just in case, to avoid CTD
				}

				if (Collision_AllowTinyCalamityCrush(giant, tiny)) {
					CrushManager::Crush(giant, tiny);
					CrushBonuses(giant, tiny); // common.hpp

					Runtime::PlaySound("GtsCrushSound", giant, 1.0, 1.0);
                    Runtime::PlaySound("GtsCrushSound", giant, 1.0, 0.0);

					Cprint("{} was instantly turned into mush by the body of {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
                    shake_camera(giant, 4.75, 0.45);
                    RefreshDuration(giant);
				} else {
					PushForward(giant, tiny, 1000);
					AddSMTDuration(giant, 2.5);
					StaggerActor(giant);

					DamageAV(tiny, ActorValue::kHealth, giantHp * 0.75);
					DamageAV(tiny, ActorValue::kHealth, giantHp * 0.25);

					shake_camera(giant, 4.35, 0.5);
					Runtime::PlaySound("lJumpLand", giant, 1.0, 1.0);

					Notify("{} is too tough to be crushed", tiny->GetDisplayFullName());

                    RefreshDuration(giant);
				}
			}
		}
	}

    void TinyCalamity_BonusSpeed(Actor* giant) { // Manages SMT bonus speed
		// Andy's TODO: Calc on demand rather than poll
		float cap = 1.0;
		float bonus = 1.0; 

		auto Attributes = Persistent::GetSingleton().GetData(giant);
		float Gigantism = 1.0 + SizeManager::GetSingleton().GetEnchantmentBonus(giant)/100;

		bool OwnsPerk = Runtime::HasPerk(giant, "NoSpeedLoss");
		float& currentspeed = Attributes->smt_run_speed;

        log::info("Current Speed of {} is {}", giant->GetDisplayFullName(), currentspeed);

		if (giant->AsActorState()->IsSprinting() && HasSMT(giant)) { // SMT Active and sprinting
			if (OwnsPerk) {
				bonus = 1.5;
				cap = 1.25;
			}

			currentspeed += 0.003600 * bonus * Gigantism; // increase MS

			if (currentspeed > cap) {
				currentspeed = cap;
			}
		} else { // else decay bonus speed over time
			if (currentspeed > 0.0) {
				currentspeed -= 0.045000;
			} else if (currentspeed <= 0.0) {
				currentspeed = 0.0;
			} 
		}
    }
}