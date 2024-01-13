#include "managers/damage/TinyCalamity.hpp"
#include "managers/CrushManager.hpp"
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
        if (Runtime::HasPerk(Giant, "NoSpeedLoss")) {
            AttributeManager::GetSingleton().OverrideSMTBonus(0.65); // Reduce speed after crush
        } else {
            AttributeManager::GetSingleton().OverrideSMTBonus(0.35); // Reduce more speed after crush
        }
    }

    bool Collision_AllowTinyCalamityCrush(Actor* giant, Actor* tiny) {
        float GiantHp = GetAV(giant, ActorValue::kHealth);
		float TinyHp = GetAV(tiny, ActorValue::kHealth);

        float Multiplier = GetSizeDifference(Giant, Tiny);
        float reduction = AttributeManager::GetSingleton().GetAttributeBonus(Giant, ActorValue::kHealth);

        if (GiantHp >= ((TinyHp / Multiplier) * reduction)) {
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

            const float BASE_DISTANCE = 24.0;
            float CheckDistance = BASE_DISTANCE*giantScale;

            if (IsDebugEnabled() && (giant->formID == 0x14 || IsTeammate(giant))) {
                DebugAPI::DrawSphere(glm::vec3(NodePosition.x, NodePosition.y, NodePosition.z), CheckDistance, 100, {0.0, 1.0, 1.0, 1.0});
            }

            NiPoint3 giantLocation = giant->GetPosition();
            for (auto otherActor: find_actors()) {
                if (otherActor != giant) {
                    NiPoint3 actorLocation = otherActor->GetPosition();
                    if ((actorLocation - giantLocation).Length() < BASE_DISTANCE*giantScale*radius*3) {
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

    void TinyCalamity_CrushCheck(Actor* Giant, Actor* Tiny) {
		auto profiler = Profilers::Profile("TinyCalamity: CrushCheck");
		if (Giant == Tiny) {
			return;
		}
		auto& persistent = Persistent::GetSingleton();
		if (persistent.GetData(Giant)) {
			if (persistent.GetData(Giant)->smt_run_speed >= 1.0) {
                float GiantHp = GetAV(giant, ActorValue::kHealth);

                if (CrushManager::AlreadyCrushed(Tiny)) {
                    return;
                }

				if (GiantHp <= 0) {
					return; // just in case, to avoid CTD
				}

				if (Collision_AllowTinyCalamityCrush(Giant, Tiny)) {
					CrushManager::Crush(Giant, Tiny);
					CrushBonuses(Giant, Tiny); // common.hpp

					Runtime::PlaySound("GtsCrushSound", Giant, 1.0, 1.0);
                    Runtime::PlaySound("GtsCrushSound", Giant, 1.0, 0.0);

					Cprint("{} was instantly turned into mush by the body of {}", Tiny->GetDisplayFullName(), Giant->GetDisplayFullName());
                    shake_camera(Giant, 1.75, 0.45);
                    RefreshDuration(giant);
				} else {
					PushForward(Giant, Tiny, 1000);
					AddSMTDuration(Giant, 2.5);
					StaggerActor(Giant);

					DamageAV(tiny, ActorValue::kHealth, GiantHp * 0.75);
					DamageAV(tiny, ActorValue::kHealth, GiantHp * 0.25);

					shake_camera(Giant, 4.35, 0.5);
					Runtime::PlaySound("lJumpLand", Giant, 1.0, 1.0);

					Notify("{} is too tough to be crushed", Tiny->GetDisplayFullName());

                    RefreshDuration(giant);
				}
			}
		}
	}
}