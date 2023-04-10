#include "managers/damage/AccurateDamage.hpp"
#include "managers/GtsSizeManager.hpp"
#include "utils/papyrusUtils.hpp"
#include "managers/explosion.hpp"
#include "managers/footstep.hpp"
#include "utils/actorUtils.hpp"
#include "utils/findActor.hpp"
#include "data/persistent.hpp"
#include "data/transient.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "data/re.hpp"
#include "node.hpp"

using namespace RE;
using namespace Gts;

namespace {
	float ShakeStrength(Actor* Source) {
		float Size = get_visual_scale(Source);
		float k = 0.065;
		float n = 1.0;
		float s = 1.12;
		float Result = 1.0/(pow(1.0+pow(k*(Size-1.0),n*s),1.0/s));
		return Result;
	}

	ExtraDataList* CreateExDataList() {
		size_t a_size;
		if (SKYRIM_REL_CONSTEXPR (REL::Module::IsAE()) && (REL::Module::get().version() >= SKSE::RUNTIME_SSE_1_6_629)) {
			a_size = 0x20;
		} else {
			a_size = 0x18;
		}
		auto memory = RE::malloc(a_size);
		std::memset(memory, 0, a_size);
		if (SKYRIM_REL_CONSTEXPR (REL::Module::IsAE()) && (REL::Module::get().version() >= SKSE::RUNTIME_SSE_1_6_629)) {
			// reinterpret_cast<std::uintptr_t*>(memory)[0] = a_vtbl; // Unknown vtable location add once REd
			REL::RelocateMember<BSReadWriteLock>(memory, 0x18) = BSReadWriteLock();
		} else {
			REL::RelocateMember<BSReadWriteLock>(memory, 0x10) = BSReadWriteLock();
		}
		return static_cast<ExtraDataList*>(memory);
	}
}

RE::ExtraDataList::~ExtraDataList() {
}

namespace Gts {
	void PlayAnimation(Actor* actor, std::string_view animName) {
		actor->NotifyAnimationGraph(animName);
	}

	void TransferInventory(Actor* from, Actor* to, bool keepOwnership, bool removeQuestItems) {
		for (auto &[a_object, invData]: from->GetInventory()) {
			from->RemoveItem(a_object, 1, ITEM_REMOVE_REASON::kRemove, nullptr, to, nullptr, nullptr);
		}
	}

	void Disintegrate(Actor* actor) {
		actor->GetActorRuntimeData().criticalStage.set(ACTOR_CRITICAL_STAGE::kDisintegrateEnd);
		actor->Disable();
		actor->SetDelete(true);
	}

	void UnDisintegrate(Actor* actor) {
		actor->GetActorRuntimeData().criticalStage.reset(ACTOR_CRITICAL_STAGE::kDisintegrateEnd);
	}

	void SetRestrained(Actor* actor) {
		CallFunctionOn(actor, "Actor", "SetRestrained", true);
	}

	void SetUnRestrained(Actor* actor) {
		CallFunctionOn(actor, "Actor", "SetRestrained", false);
	}

	void SetDontMove(Actor* actor) {
		CallFunctionOn(actor, "Actor", "SetDontMove", true);
	}

	void SetMove(Actor* actor) {
		CallFunctionOn(actor, "Actor", "SetDontMove", true);
	}

	std::vector<hkpRigidBody*> GetActorRB(Actor* actor) {
		std::vector<hkpRigidBody*> results = {};
		auto charCont = actor->GetCharController();
		if (!charCont) {
			return results;
		}

		bhkCharProxyController* charProxyController = skyrim_cast<bhkCharProxyController*>(charCont);
		bhkCharRigidBodyController* charRigidBodyController = skyrim_cast<bhkCharRigidBodyController*>(charCont);
		if (charProxyController) {
			// Player controller is a proxy one
			auto& proxy = charProxyController->proxy;
			hkReferencedObject* refObject = proxy.referencedObject.get();
			if (refObject) {
				hkpCharacterProxy* hkpObject = skyrim_cast<hkpCharacterProxy*>(refObject);

				if (hkpObject) {
					// Not sure what bodies is doing
					for (auto body: hkpObject->bodies) {
						results.push_back(body);
					}
					// // This one appears to be active during combat.
					// // Maybe used for sword swing collision detection
					// for (auto phantom: hkpObject->phantoms) {
					// 	results.push_back(phantom);
					// }
					//
					// // This is the actual shape
					// if (hkpObject->shapePhantom) {
					// 	results.push_back(hkpObject->shapePhantom);
					// }
				}
			}
		} else if (charRigidBodyController) {
			// NPCs seem to use rigid body ones
			auto& characterRigidBody = charRigidBodyController->characterRigidBody;
			hkReferencedObject* refObject = characterRigidBody.referencedObject.get();
			if (refObject) {
				hkpCharacterRigidBody* hkpObject = skyrim_cast<hkpCharacterRigidBody*>(refObject);
				if (hkpObject) {
					if (hkpObject->m_character) {
						results.push_back(hkpObject->m_character);
					}
				}
			}
		}

		return results;
	}

	void PushActorAway(TESObjectREFR* source, Actor* receiver, float afKnockBackForce) {
		/*NiPoint3 sourceLoc = source->GetPosition();
		   NiPoint3 destinationLoc = receiver->GetPosition();
		   NiPoint3 direction = destinationLoc - sourceLoc;
		   NiPoint3 niImpulse  = direction * afKnockBackForce/direction.Length();
		   hkVector4 impulse = hkVector4(niImpulse.x, niImpulse.y, niImpulse.z, 1.0);

		   auto rbs = GetActorRB(receiver);
		   for (auto rb: rbs) {
		        auto& motion = rb->motion;
		        motion.ApplyLinearImpulse(impulse);
		        log::info("Trying to push actor {} away", receiver);
		   }*/
		if (receiver->IsDead()) {
			return;
		}
		CallFunctionOn(source, "ObjectReference", "PushActorAway", receiver, afKnockBackForce);
	}
	void KnockAreaEffect(TESObjectREFR* source, float afMagnitude, float afRadius) {
		// NiPoint3 sourceLoc = source->GetPosition();
		// for (auto actor: find_actors()) {
		// 	if ((actor->GetPosition() - sourceLoc).Length() < afRadius) {
		// 		auto aiProc = actor->currentProcess;
		// 		if (aiProc) {
		// 			aiProc->KnockExplosion(actor, sourceLoc, afMagnitude);
		// 		}
		// 	}
		// }
		CallFunctionOn(source, "ObjectReference", "KnockAreaEffect", afMagnitude, afRadius);
	}
	void ApplyHavokImpulse(TESObjectREFR* target, float afX, float afY, float afZ, float afMagnitude) {
		/*NiPoint3 direction = NiPoint3(afX, afY, afZ);
		   NiPoint3 niImpulse = direction * afMagnitude/direction.Length();
		   hkVector4 impulse = hkVector4(niImpulse.x, niImpulse.y, niImpulse.z, 0.0);//hkVector4(niImpulse.x, niImpulse.y, niImpulse.z, 0.0);

		   auto rbs = GetActorRB(target);
		   for (auto rb: rbs) {
		        auto& motion = rb->motion;
		        motion.ApplyLinearImpulse(impulse);
		   }*/
		CallFunctionOn(target, "ObjectReference", "ApplyHavokImpulse", afX, afY, afZ, afMagnitude);
	}

	void CompleteDragonQuest() {
		auto pc = PlayerCharacter::GetSingleton();
		auto progressionQuest = Runtime::GetQuest("MainQuest");
		if (progressionQuest) {
			auto stage = progressionQuest->GetCurrentStageID();
			if (stage == 90) {
				auto transient = Transient::GetSingleton().GetData(pc);
				if (transient) {
					ConsoleLog::GetSingleton()->Print("Quest is Completed");
					transient->dragon_was_eaten = true;
				}
			}
		}
	}

	bool IsDragon(Actor* actor) {
		if ( std::string(actor->GetDisplayFullName()).find("ragon") != std::string::npos
			|| std::string(actor->GetDisplayFullName()).find("Dragon") != std::string::npos
			|| std::string(actor->GetDisplayFullName()).find("dragon") != std::string::npos
			|| Runtime::IsRace(actor, "dragonRace")
			)
			{
				return true;
			}
			else {
				return false;
			}
	}

	bool IsProne(Actor* actor) {
		return actor!= nullptr && actor->formID == 0x14 && Runtime::GetBool("ProneEnabled") && actor->AsActorState()->IsSneaking();
	}

	bool IsJumping(Actor* actor) {
		if (!actor) {
			return false;
		}
		if (!actor->Is3DLoaded()) {
			return false;
		}
		bool result = false;
		actor->GetGraphVariableBool("bInJumpState", result);
		return result;
	}

	float get_distance_to_actor(Actor* receiver, Actor* target) {
		if (target) {
			auto point_a = receiver->GetPosition();
			auto point_b = target->GetPosition();
			auto delta = point_a - point_b;
			return delta.Length();
		}
		return 3.4028237E38; // Max float
	}

	void ApplyShake(Actor* caster, float modifier) {
		if (caster) {
			auto position = caster->GetPosition();
			ApplyShakeAtPoint(caster, modifier, position);
		}
	}

	void ApplyShakeAtNode(Actor* caster, float modifier, std::string_view nodesv) {
		auto node = find_node(caster, nodesv);
		if (node) {
			ApplyShakeAtPoint(caster, modifier, node->world.translate);
		}
	}
	void ApplyShakeAtPoint(Actor* caster, float modifier, const NiPoint3& coords) {
		if (!caster) {
			return;
		}
		// Reciever is always PC if it is not PC we do nothing anyways
		Actor* receiver = PlayerCharacter::GetSingleton();
		if (!receiver) {
			return;
		}

		float distance = get_distance_to_camera(coords);
		float sourcesize = get_visual_scale(caster);
		float receiversize = get_visual_scale(receiver);
		float sizedifference = sourcesize/receiversize;
		if (caster->formID == 0x14) {
			sizedifference = sourcesize;
		}


		// To Sermit: You wrote a cutoff not a falloff
		//            was this intentional?
		//
		// FYI: This is the difference
		// Falloff:
		//   |
		// I |----\
		//   |     \
		//   |______\___
		//        distance
		// Cuttoff:
		//   |
		// I |----|
		//   |    |
		//   |____|_____
		//        distance
		float cuttoff = 450 * sizedifference;
		//log::info("Shake Actor:{}, Distance:{}, sourcesize: {}, recsize: {}, cutoff: {}", caster->GetDisplayFullName(), distance, sourcesize, receiversize, cuttoff);
		if (distance < cuttoff) {
			// To Sermit: Same value as before just with the math reduced to minimal steps
			float intensity = (sizedifference * 17.9 * ShakeStrength(caster)) / distance;
			float duration = 0.25 * intensity * (1 + (sizedifference * 0.25));
			intensity = std::clamp(intensity, 0.0f, 1e8f);
			duration = std::clamp(duration, 0.0f, 1.2f);

			shake_controller(intensity*modifier, intensity*modifier, duration);
			shake_camera_at_node(coords, intensity*modifier, duration);
		}
	}

	void EnableFreeCamera() {
		auto playerCamera = PlayerCamera::GetSingleton();
		playerCamera->ToggleFreeCameraMode(false);
	}

	bool AllowDevourment() {
		return Persistent::GetSingleton().devourment_compatibility;
	}

	bool AllowFeetTracking() {
		return Persistent::GetSingleton().allow_feetracking;
	}

	void TrackFeet(Actor* giant, float number, bool enable) {
		if (giant->formID == 0x14) {
			if (AllowFeetTracking()) {
				auto& sizemanager = SizeManager::GetSingleton();
				sizemanager.SetActionBool(giant, enable, number);
			}
		}
	}

	void CallDevourment(Actor* giant, Actor* tiny) {
		auto progressionQuest = Runtime::GetQuest("MainQuest");
		if (progressionQuest) {
			CallFunctionOn(progressionQuest, "gtsProgressionQuest", "Devourment", giant, tiny);
		}
	}

	void CallGainWeight(Actor* giant, float value) {
		auto progressionQuest = Runtime::GetQuest("MainQuest");
		if (progressionQuest) {
			CallFunctionOn(progressionQuest, "gtsProgressionQuest", "GainWeight", giant, value);
		}
	}

	void CallVampire() {
		auto progressionQuest = Runtime::GetQuest("MainQuest");
		if (progressionQuest) {
			CallFunctionOn(progressionQuest, "gtsProgressionQuest", "SatisfyVampire");
		}
	}

	void PerkPointCheck(float level) {
		auto progressionQuest = Runtime::GetQuest("MainQuest");
		int bonus = 1.0;
		if (level == 5   || level == 10 || level == 15 || level == 20  ||
		    level == 25  || level == 30 || level == 35 || level == 40  ||
			level == 45  || level == 50 || level == 55 || level == 60  ||
			level == 65  || level == 70 || level == 75 || level == 80  ||
			level == 85  || level == 90 || level == 95 || level == 100 
 		) {
			if (progressionQuest) {
				CallFunctionOn(progressionQuest, "gtsProgressionQuest", "GainPerkPoint", bonus);
			}
		}
	}

	void DoSizeEffect(Actor* giant, float modifier, FootEvent kind, std::string_view node) {
		auto& footstep = FootStepManager::GetSingleton();
		auto& explosion = ExplosionManager::GetSingleton();
		Impact impact_data = Impact {
			.actor = giant,
			.kind = kind,
			.scale = get_visual_scale(giant) * modifier,
			.effective_scale = get_effective_scale(giant),
			.nodes = find_node(giant, node),
		};
		explosion.OnImpact(impact_data); // Play explosion
		footstep.OnImpact(impact_data); // Play sound
	}

	void DoDamageEffect(Actor* giant, float damage, float radius, int random, float bonedamage) {
		float damage_mult = Persistent::GetSingleton().size_related_damage_mult;
		AccurateDamage::GetSingleton().DoAccurateCollision(giant, (35.0 * damage * damage_mult), radius, random, bonedamage);
	}

	hkaRagdollInstance* GetRagdoll(Actor* actor) {
		BSAnimationGraphManagerPtr animGraphManager;
		if (actor->GetAnimationGraphManager(animGraphManager)) {
			for (auto& graph : animGraphManager->graphs) {
				if (graph) {
					auto& character = graph->characterInstance;
					auto ragdollDriver = character.ragdollDriver.get();
					if (ragdollDriver) {
						auto ragdoll = ragdollDriver->ragdoll;
						if (ragdoll) {
							return ragdoll;
						}
					}
				}
			}
		}
		return nullptr;
	}

	void ManageRagdoll(Actor* tiny, float deltaLength, NiPoint3 deltaLocation, NiPoint3 targetLocation) {
		if (deltaLength >= 70.0) {
            // WARP if > 1m
            auto ragDoll = GetRagdoll(tiny);
            hkVector4 delta = hkVector4(deltaLocation.x/70.0, deltaLocation.y/70.0, deltaLocation.z/70, 1.0);
            for (auto rb: ragDoll->rigidBodies) {
              if (rb) {
                auto ms = rb->GetMotionState();
                if (ms) {
                  hkVector4 currentPos = ms->transform.translation;
                  hkVector4 newPos = currentPos + delta;
                  rb->motion.SetPosition(newPos);
                  rb->motion.SetLinearVelocity(hkVector4(0.0, 0.0, -10.0, 0.0));
                }
              }
            }
          } else {
            // Just move the hand if <1m
            std::string_view handNodeName = "NPC HAND L [L Hand]";
            auto handBone = find_node(tiny, handNodeName);
            if (handBone) {
              	auto collisionHand = handBone->GetCollisionObject();
              	if (collisionHand) {
                	auto handRbBhk = collisionHand->GetRigidBody();
               		if (handRbBhk) {
                  		auto handRb = static_cast<hkpRigidBody*>(handRbBhk->referencedObject.get());
                  		if (handRb) {
                    		auto ms = handRb->GetMotionState();
                    		if (ms) {
                     			hkVector4 targetLocationHavok = hkVector4(targetLocation.x/70.0, targetLocation.y/70.0, targetLocation.z/70, 1.0);
                      			handRb->motion.SetPosition(targetLocationHavok);
                      			handRb->motion.SetLinearVelocity(hkVector4(0.0, 0.0, -10.0, 0.0));
                    		}
                  		}
                	}
              	}
            }
        }
	}

	void PrintDeathSource(Actor* giant, Actor* tiny, std::string_view cause) {
		int random = rand()% 8;
		float sizedifference = get_visual_scale(giant)/get_visual_scale(tiny);
		if (cause == "Crushed") { // Default crush
			if (random == 1) {
				ConsoleLog::GetSingleton()->Print("%s became a bloody stain under %s foot.", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random <= 2) {
				ConsoleLog::GetSingleton()->Print("%s was crushed by the feet of %s", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random == 4) {
				ConsoleLog::GetSingleton()->Print("Feet of %s crushed %s into nothing", giant->GetDisplayFullName(), tiny->GetDisplayFullName());
			} else if (random >= 6) {
				ConsoleLog::GetSingleton()->Print("%s Got crushed by %s", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random >= 7) {
				ConsoleLog::GetSingleton()->Print("%s relentlessly crushed %s", giant->GetDisplayFullName(), tiny->GetDisplayFullName());
			}
			return;
		}
		else if (cause == "HandCrushed") {
			if (random == 1) {
				ConsoleLog::GetSingleton()->Print("%s life was squeezed out in %s grip", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random <= 2) {
				ConsoleLog::GetSingleton()->Print("%s was crushed between the fingers of %s", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random == 4) {
				ConsoleLog::GetSingleton()->Print("%s has been crushed in the hand of %s", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random >= 6) {
				ConsoleLog::GetSingleton()->Print("%s applied too much pressure to her hand, crushing %s", giant->GetDisplayFullName(), tiny->GetDisplayFullName());
			} else if (random >= 7) {
				ConsoleLog::GetSingleton()->Print("%s was turned into nothing inside the hand of %s", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			}
			return;
		}
		else if (cause == "Shrinked") {
			if (random <= 2) {
				ConsoleLog::GetSingleton()->Print("%s greedily absorbed %s", giant->GetDisplayFullName(), tiny->GetDisplayFullName());
			} else if (random >= 4) {
				ConsoleLog::GetSingleton()->Print("%s completely absorbed %s", giant->GetDisplayFullName(), tiny->GetDisplayFullName());
			} else if (random >= 6) {
				ConsoleLog::GetSingleton()->Print("%s was absorbed by %s", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random >= 7) {
				ConsoleLog::GetSingleton()->Print("%s was shrinkned to nothing by %s", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			}
			return;
		}
		else if (cause == "Vored") {
			return;
		}
		else if (cause == "ThighCrushed") {
			if (random == 1) {
				ConsoleLog::GetSingleton()->Print("%s was crushed to death between %s thighs.", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} if (random <= 3) {
				ConsoleLog::GetSingleton()->Print("%s crushed %s during leg stretch", giant->GetDisplayFullName(), tiny->GetDisplayFullName());
			} else if (random == 4) {
				ConsoleLog::GetSingleton()->Print("%s ended life of %s between legs", giant->GetDisplayFullName(), tiny->GetDisplayFullName());
			} else if (random == 5) {
				ConsoleLog::GetSingleton()->Print("%s applied too much leg pressure to %s", giant->GetDisplayFullName(), tiny->GetDisplayFullName());
			} else if (random >= 6) {
				ConsoleLog::GetSingleton()->Print("%s was shrinkned to nothing by %s", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			}
			return;
			}
		else if (cause == "ThighSandwiched") {
			if (random <= 3) {
				ConsoleLog::GetSingleton()->Print("%s was crushed by the thighs of %s", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random == 4) {
				ConsoleLog::GetSingleton()->Print("Thighs of %s gently crushed %s", giant->GetDisplayFullName(), tiny->GetDisplayFullName());
			} else if (random == 5) {
				ConsoleLog::GetSingleton()->Print("%s has disappeared between the thighs of %s", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random == 6) {
				ConsoleLog::GetSingleton()->Print("%s was smothered to nothing between the thighs of %s", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random >= 7) {
				ConsoleLog::GetSingleton()->Print("Thighs of %s sandwiched %s to nothing", giant->GetDisplayFullName(), tiny->GetDisplayFullName());
			}
			return;
		}
	}
}
