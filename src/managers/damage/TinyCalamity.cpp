#include "managers/damage/tinyCalamity.hpp"
#include "managers/CrushManager.hpp"
#include "magic/effects/common.hpp"
#include "managers/Attributes.hpp"
#include "utils/actorUtils.hpp"
#include "data/persistent.hpp"
#include "utils/looting.hpp"
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
    void PlayGoreEffects(Actor* giant, Actor* tiny) {
        if (!IsLiving(tiny)) {
            SpawnDustParticle(tiny, giant, "NPC Root [Root]", 3.0);
        } else {
            if (!LessGore()) {
                auto root = find_node(tiny, "NPC Root [Root]");
                if (root) {
                    float currentSize = get_visual_scale(tiny);
                    SpawnParticle(tiny, 0.60, "GTS/Damage/Explode.nif", root->world.rotate, root->world.translate, currentSize * 1.25, 7, root);
                    SpawnParticle(tiny, 0.60, "GTS/Damage/Explode.nif", root->world.rotate, root->world.translate, currentSize * 1.25, 7, root);
                    SpawnParticle(tiny, 0.60, "GTS/Damage/Crush.nif", root->world.rotate, root->world.translate, currentSize * 1.25, 7, root);
                    SpawnParticle(tiny, 0.60, "GTS/Damage/Crush.nif", root->world.rotate, root->world.translate, currentSize * 1.25, 7, root);
                    SpawnParticle(tiny, 1.20, "GTS/Damage/ShrinkOrCrush.nif", NiMatrix3(), root->world.translate, currentSize * 12.5, 7, root);
                }
            }
            Runtime::PlayImpactEffect(tiny, "GtsBloodSprayImpactSet", "NPC Root [Root]", NiPoint3{0, 0, -1}, 512, false, true);
            Runtime::PlayImpactEffect(tiny, "GtsBloodSprayImpactSet", "NPC Root [Root]", NiPoint3{0, 0, -1}, 512, false, true);
            Runtime::PlayImpactEffect(tiny, "GtsBloodSprayImpactSet", "NPC Root [Root]", NiPoint3{0, 0, -1}, 512, false, true);
            Runtime::CreateExplosion(tiny, get_visual_scale(tiny) * 0.5, "BloodExplosion");
        }
    }

    void MoveItems(ActorHandle giantHandle, ActorHandle tinyHandle, FormID ID) {
        std::string taskname = std::format("CollisionDeath {}", ID);
        TaskManager::RunOnce(taskname, [=](auto& update){
            if (!tinyHandle) {
                return;
            }
            if (!giantHandle) {
                return;
            }
            auto giant = giantHandle.get().get();
            auto tiny = tinyHandle.get().get();
            float scale = get_visual_scale(tiny);
            TransferInventory(tiny, giant, scale, false, true, DamageSource::Collision, true);
        });
    }

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
    void TinyCalamity_ExplodeActor(Actor* giant, Actor* tiny) {
        if (!tiny->IsDead()) {
            KillActor(giant, tiny);
        }

        ActorHandle giantHandle = giant->CreateRefHandle();
        ActorHandle tinyHandle = tiny->CreateRefHandle();

        CrushBonuses(giant, tiny);                             // common.hpp
        PlayGoreEffects(tiny, giant);    
        MoveItems(giantHandle, tinyHandle, tiny->formID);
        PrintDeathSource(giant, tiny, DamageSource::Collision);

        float OldScale;
        giant->GetGraphVariableFloat("GiantessScale", OldScale); // save old scale
        giant->SetGraphVariableFloat("GiantessScale", 1.0); // Needed to allow Stagger to play, else it won't work

        shake_camera(giant, 8.0, 0.45);
        RefreshDuration(giant);

        giant->SetGraphVariableFloat("staggerMagnitude", 0.50f);
		giant->NotifyAnimationGraph("staggerStart");

        Runtime::PlaySound("GtsCrushSound", giant, 1.0, 0.0);

        if (tiny->formID != 0x14) {
            Disintegrate(tiny, true); // Set critical stage 4 on actors
        } else if (tiny->formID == 0x14) {
            TriggerScreenBlood(50);
            tiny->SetAlpha(0.0); // Player can't be disintegrated, so we make player Invisible
        }

        giant->SetGraphVariableFloat("GiantessScale", OldScale);
    }

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

				if (giantHp <= 0) {
					return; // just in case, to avoid CTD
				}

				if (Collision_AllowTinyCalamityCrush(giant, tiny)) {
                    TinyCalamity_ExplodeActor(giant, tiny);
				} else {
                    float OldScale;
                    giant->GetGraphVariableFloat("GiantessScale", OldScale); // save old scale
                    giant->SetGraphVariableFloat("GiantessScale", 1.0); // Needed to allow Stagger to play, else it won't work

					PushForward(giant, tiny, 1000);
					AddSMTDuration(giant, 2.5);
					StaggerActor(giant);

					DamageAV(tiny, ActorValue::kHealth, giantHp * 0.75);
					DamageAV(tiny, ActorValue::kHealth, giantHp * 0.25);

					shake_camera(giant, 4.35, 0.5);
					Runtime::PlaySound("lJumpLand", giant, 1.0, 1.0);

					Notify("{} is too tough to be crushed", tiny->GetDisplayFullName());

                    RefreshDuration(giant);

                    giant->SetGraphVariableFloat("GiantessScale", OldScale);
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
				currentspeed -= 0.045000 / bonus;
			} else if (currentspeed <= 0.0) {
				currentspeed = 0.0;
			} 
		}
    }
}