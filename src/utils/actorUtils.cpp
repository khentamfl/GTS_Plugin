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
#include "timer.hpp"
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
			if (a_object->GetPlayable()) {
				if (!invData.second->IsQuestObject() || removeQuestItems ) {
					from->RemoveItem(a_object, 1, ITEM_REMOVE_REASON::kRemove, nullptr, to, nullptr, nullptr);
				}
			}
		}
	}

	void Disintegrate(Actor* actor) {
		actor->GetActorRuntimeData().criticalStage.set(ACTOR_CRITICAL_STAGE::kDisintegrateEnd);
		actor->Disable();
		if (Persistent::GetSingleton().delete_actors) {
			actor->SetDelete(true);
			log::info("Calling Delete Actors");
		}
	}

	void UnDisintegrate(Actor* actor) {
		actor->GetActorRuntimeData().criticalStage.reset(ACTOR_CRITICAL_STAGE::kDisintegrateEnd);
	}

	void StartCombat(Actor* giant, Actor* tiny, bool Forced) {
		if (!Runtime::GetBool("HostileDamage")) {
			return;
		}
		static Timer tick = Timer(0.25);
		if (tick.ShouldRunFrame() || Forced == true) {
			if (tiny->IsInCombat() || tiny->IsDead()) {
				return;
			}
			if (Forced == true || GetAV(tiny, ActorValue::kHealth) < GetMaxAV(tiny, ActorValue::kHealth) * 0.90) {
				CallFunctionOn(tiny, "Actor", "StartCombat", giant);
			}
		}
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
		if (receiver->IsDead()) {
			return;
		}
		CallFunctionOn(source, "ObjectReference", "PushActorAway", receiver, afKnockBackForce);
	}
	void KnockAreaEffect(TESObjectREFR* source, float afMagnitude, float afRadius) {
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
					Cprint("Quest is Completed");
					transient->dragon_was_eaten = true;
				}
			}
		}
	}

	bool IsDragon(Actor* actor) {
		if ( std::string(actor->GetDisplayFullName()).find("ragon") != std::string::npos
		     || std::string(actor->GetDisplayFullName()).find("Dragon") != std::string::npos
		     || std::string(actor->GetDisplayFullName()).find("dragon") != std::string::npos
		     || Runtime::IsRace(actor, "dragonRace")) {
			return true;
		} else {
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
			ApplyShakeAtPoint(caster, modifier, position, 1.0);
		}
	}

	void ApplyShakeAtNode(Actor* caster, float modifier, std::string_view nodesv) {
		auto node = find_node(caster, nodesv);
		if (node) {
			ApplyShakeAtPoint(caster, modifier, node->world.translate, 1.0);
		}
	}

	void ApplyShakeAtNode(Actor* caster, float modifier, std::string_view nodesv, float radius) {
		auto node = find_node(caster, nodesv);
		if (node) {
			ApplyShakeAtPoint(caster, modifier, node->world.translate, radius);
		}
	}

	void ApplyShakeAtPoint(Actor* caster, float modifier, const NiPoint3& coords, float radius) {
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
		float cuttoff = 450 * sizedifference * radius;
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

	bool IsGtsBusy(Actor* actor) {
		bool GTSBusy;
		actor->GetGraphVariableBool("GTS_Busy", GTSBusy);
		return GTSBusy;
	}

	bool IsTeammate(Actor* actor) {
		if (Runtime::InFaction(actor, "FollowerFaction") || actor->IsPlayerTeammate()) {
			return true;
		}
		return false;
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

	void CallHelpMessage() {
		auto progressionQuest = Runtime::GetQuest("MainQuest");
		if (progressionQuest) {
			CallFunctionOn(progressionQuest, "gtsProgressionQuest", "TrueGiantessMessage");
		}
	}

	void PerkPointCheck(float level) {
		auto progressionQuest = Runtime::GetQuest("MainQuest");
		int bonus = 1.0;
		if (int(level) % 5 == 0) {
			if (progressionQuest) {
				CallFunctionOn(progressionQuest, "gtsProgressionQuest", "GainPerkPoint", bonus);
			}
		}
	}

	float GetRandomBoost() {
		float rng = (rand()% 150 + 1);
		float random = rng/100;
		return random;
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

	void SpawnParticle(Actor* actor, float lifetime, const char* modelName, const NiMatrix3& rotation, const NiPoint3& position, float scale, std::uint32_t flags, NiAVObject* target) {
		auto cell = actor->GetParentCell();
		if (cell) {
			BSTempEffectParticle::Spawn(cell, lifetime, modelName, rotation, position, scale, flags, target);
		}
	}

	void DoDamageEffect(Actor* giant, float damage, float radius, int random, float bonedamage) {
		float damagebonus = Persistent::GetSingleton().size_related_damage_mult;
		AccurateDamage::GetSingleton().DoAccurateCollision(giant, (30.0 * damage * damagebonus), radius, random, bonedamage);
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

	void StaggerActor(Actor* receiver) {
		receiver->SetGraphVariableFloat("staggerMagnitude", 100.00f);
		receiver->NotifyAnimationGraph("staggerStart");
	}

	float GetMovementModifier(Actor* giant) {
		if (giant->AsActorState()->IsSprinting()) {
			return 2.0;
		} else if (giant->AsActorState()->IsSneaking()) {
			return 0.6;
		} else {
			return 1.0;
		}
	}

	void ScareActors(Actor* giant) {
		Profilers::Start("ActorUtils: ScareActors");
		//log::info("Original Giant is {}", giant->GetDisplayFullName());
		for (auto tiny: find_actors()) {
			if (tiny != giant && tiny->formID != 0x14 && !IsTeammate(tiny)) {
				 if (tiny->IsDead()) {
					//log::info("{} is dead", tiny->GetDisplayFullName());
					return;
				}
				//log::info("Giant is {}", giant->GetDisplayFullName());
				float random = GetRandomBoost();
				static Timer runtimer = Timer(1.0);
				static Timer combattimer = Timer(2.0);
				float GiantScale = get_visual_scale(giant);
				float TinyScale = get_visual_scale(tiny);
				float sizedifference = std::clamp(GiantScale/TinyScale, 0.10f, 12.0f);
				float distancecheck = 226.0 * GetMovementModifier(giant);
				if (sizedifference >= 2.5) {
					//log::info("Size Difference of {} is > than x2.5", giant->GetDisplayFullName());
					NiPoint3 GiantDist = giant->GetPosition();
					NiPoint3 ObserverDist = tiny->GetPosition();
					float distance = (GiantDist - ObserverDist).Length();
					//log::info("Distance between {} and {}, distance: {}, check: {}", giant->GetDisplayFullName(), tiny->GetDisplayFullName(), distance, distancecheck);

					if (distance <= distancecheck * sizedifference) {
						//log::info("Distance of {} < Check, applying Flee", giant->GetDisplayFullName());
						auto combat = tiny->GetActorRuntimeData().combatController;
						tiny->GetActorRuntimeData().currentCombatTarget = giant->CreateRefHandle();
						auto TinyRef = skyrim_cast<TESObjectREFR*>(tiny);

						if (TinyRef) {
							auto GiantRef = skyrim_cast<TESObjectREFR*>(giant);
							if (GiantRef) {
								//log::info("GiantRef {} true", giant->GetDisplayFullName());
								bool SeeingOther;
								bool IsTrue = tiny->HasLineOfSight(GiantRef, SeeingOther);
								if (IsTrue || distance < (distancecheck/1.5) * sizedifference) {
									//log::info("Distance True");
									auto cell = tiny->GetParentCell();
									if (cell) {
										if (runtimer.ShouldRunFrame()) {
											if (!combat) {
												//log::info("Combat false, applying Flee");
												StartCombat(tiny, tiny, true);
												tiny->InitiateFlee(TinyRef, true, true, true, cell, TinyRef, 100.0, 465.0 * sizedifference);
											} 
										} if (combattimer.ShouldRunFrame() && combat && GetRandomBoost() <= 0.040 * (sizedifference)) {
											std::vector<Actor*> FearList = {};
											//log::info("Combat True, applying Flee");
											FearList.push_back(tiny);
											if (!FearList.empty()) {
												int idx = rand() % FearList.size();
												Actor* FearReceiver = FearList[idx];
												if (FearReceiver) {
													auto ReceiverRef = skyrim_cast<TESObjectREFR*>(FearReceiver);
													if (ReceiverRef) {
														FearReceiver->InitiateFlee(ReceiverRef, true, true, true, cell, ReceiverRef, 100.0, 265.0 * sizedifference);
														combat->startedCombat = true;
														combat->ignoringCombat = false;
														combat->state->isFleeing = true;
														FearList = {};
													}
												}
											}
										}
									} 
								}
							}
						}
					}
				}
			}
			Profilers::Stop("ActorUtils: ScareActors");
		}
	}
		

	void ReportCrime(Actor* giant, Actor* tiny, float value, bool combat) {
		Profilers::Start("ActorUtils: ReportCrime");
		if (!Runtime::GetBool("HostileDamage")) {
			return;
		}
		static Timer tick = Timer(0.10);
		bool SeeingOther;
		if (tick.ShouldRunFrame()) {
			for (auto otherActor: find_actors()) {
				float scale = std::clamp(get_visual_scale(giant), 0.10f, 12.0f);
				auto Ref = skyrim_cast<TESObjectREFR*>(tiny);
				if (Ref) {
					bool IsTrue = otherActor->HasLineOfSight(Ref, SeeingOther);
					NiPoint3 ObserverDist = otherActor->GetPosition();
					NiPoint3 VictimDist = tiny->GetPosition();
					float distance = (ObserverDist - VictimDist).Length();
					if (IsTrue || distance < 524 * scale) {
						if (otherActor != tiny && tiny->formID != 0x14) {
							auto Faction = tiny->GetCrimeFaction();
							auto OtherFaction = otherActor->GetCrimeFaction();
							auto CombatValue = giant->GetCrimeGoldValue(Faction);
							if (combat && OtherFaction == Faction) {
								StartCombat(giant, otherActor, true);
								tiny->GetActorRuntimeData().myKiller = giant->CreateRefHandle();
								tiny->GetActorRuntimeData().currentCombatTarget = giant->CreateRefHandle();
								tiny->UpdateCombatControllerSettings();
							} 
							else if (!combat) {
								if (giant->formID == 0x14 && CombatValue < 1000) {
									giant->ModCrimeGoldValue(Faction, true, value);
								}
							}
						}
					}
				}
			}
		}
		Profilers::Stop("ActorUtils: ReportCrime");
	}

	void PrintDeathSource(Actor* giant, Actor* tiny, std::string_view cause) {
		int random = rand()% 8;
		float sizedifference = get_visual_scale(giant)/get_visual_scale(tiny);
		if (cause == "Crushed") { // Default crush
			if (random < 2) {
				Cprint("{} became a bloody stain under {} foot.", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random == 2) {
				Cprint("{} was crushed by the feet of {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random == 3 || random == 4) {
				Cprint("Feet of {} crushed {} into nothing", giant->GetDisplayFullName(), tiny->GetDisplayFullName());
			} else if (random == 5 || random == 6) {
				Cprint("{} got crushed by {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random >= 7) {
				Cprint("{} relentlessly crushed {}", giant->GetDisplayFullName(), tiny->GetDisplayFullName());
			}
			return;
		} else if (cause == "HandCrushed") {
			if (random == 1) {
				Cprint("{} life was squeezed out in {} grip", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random <= 2) {
				Cprint("{} was crushed between the fingers of {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random == 4) {
				Cprint("{} has been crushed in the hand of {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random >= 6) {
				Cprint("{} applied too much pressure to her hand, crushing {}", giant->GetDisplayFullName(), tiny->GetDisplayFullName());
			} else if (random >= 7) {
				Cprint("{} was turned into nothing inside the hand of {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			}
			return;
		} else if (cause == "Shrinked") {
			if (random <= 2) {
				Cprint("{} greedily absorbed {}", giant->GetDisplayFullName(), tiny->GetDisplayFullName());
			} else if (random >= 4) {
				Cprint("{} completely absorbed {}", giant->GetDisplayFullName(), tiny->GetDisplayFullName());
			} else if (random >= 6) {
				Cprint("{} was absorbed by {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random >= 7) {
				Cprint("{} was shrinkned to nothing by {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			}
			return;
		} else if (cause == "Vored") {
			return;
		} else if (cause == "ThighCrushed") {
			if (random == 1) {
				Cprint("{} was crushed to death between {} thighs.", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			}
			if (random <= 3) {
				Cprint("{} crushed {} during leg stretch", giant->GetDisplayFullName(), tiny->GetDisplayFullName());
			} else if (random == 4) {
				Cprint("{} ended life of {} between legs", giant->GetDisplayFullName(), tiny->GetDisplayFullName());
			} else if (random == 5) {
				Cprint("{} applied too much leg pressure to {}", giant->GetDisplayFullName(), tiny->GetDisplayFullName());
			} else if (random >= 6) {
				Cprint("{} was shrinkned to nothing by {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			}
			return;
		} else if (cause == "ThighSandwiched") {
			if (random <= 3) {
				Cprint("{} was crushed by the thighs of {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random == 4) {
				Cprint("Thighs of {} gently crushed {}", giant->GetDisplayFullName(), tiny->GetDisplayFullName());
			} else if (random == 5) {
				Cprint("{} has disappeared between the thighs of {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random == 6) {
				Cprint("{} was smothered to nothing between the thighs of {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random >= 7) {
				Cprint("Thighs of {} sandwiched {} to nothing", giant->GetDisplayFullName(), tiny->GetDisplayFullName());
			}
			return;
		} else if (cause == "Overkill") {
			if (random <= 3) {
				Cprint("{} body exploded because of massive size difference with {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else {
				Cprint("{} hit {} with so much force that {} exploded into bloody mist", giant->GetDisplayFullName(), tiny->GetDisplayFullName(), tiny->GetDisplayFullName());
			}
		} else if (cause == "HitSteal") {
			if (random <= 2) {
				Cprint("{} body exploded after trying to hit {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random == 3) {
				Cprint("Protective magic of {} made {} absorb {}", giant->GetDisplayFullName(), giant->GetDisplayFullName(), tiny->GetDisplayFullName());
			} else if (random > 3) {
				Cprint("{} Tried to kill {}, but ended up being absorbed by the size magic of {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName(), giant->GetDisplayFullName());
			}
		} else if (cause == "Explode") {
			if (random <= 2) {
				Cprint("{} exploded into bloody dust", tiny->GetDisplayFullName());
			} else if (random == 3) {
				Cprint("{} suddenly exploded", tiny->GetDisplayFullName());
			} else if (random > 3) {
				Cprint("{} was turned into nothing", tiny->GetDisplayFullName());
			}
		}
	}

	void PrintSuffocate(Actor* pred, Actor* prey) {
		int random = rand() % 5;
		if (random <= 1) {
			Cprint("{} was slowly smothered between {} thighs", prey->GetDisplayFullName(), pred->GetDisplayFullName());
		} else if (random == 2) {
			Cprint("{} was suffocated by the thighs of {}", prey->GetDisplayFullName(), pred->GetDisplayFullName());
		} else if (random == 3) {
			Cprint("Thighs of {} suffocated {} to death", pred->GetDisplayFullName(), prey->GetDisplayFullName());
		} else if (random == 4) {
			Cprint("{} got smothered between the thighs of {}", prey->GetDisplayFullName(), pred->GetDisplayFullName());
		} else if (random == 5) {
			Cprint("{} lost life to the thighs of {}", prey->GetDisplayFullName(), pred->GetDisplayFullName());
		}
	}
}
