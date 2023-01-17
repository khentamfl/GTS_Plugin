#include "utils/actorUtils.hpp"
#include "utils/findActor.hpp"
#include "utils/papyrusUtils.hpp"
#include "data/runtime.hpp"
#include "data/re.hpp"

using namespace RE;
using namespace Gts;

namespace Gts {
	void PlayAnimation(Actor* actor, std::string_view animName) {
		actor->NotifyAnimationGraph(animName);
	}

	void TransferInventory(Actor* from, Actor* to, bool keepOwnership, bool removeQuestItems) {
		for (auto &[a_object, invData]: from->GetInventory()) {
			const auto& [count, entry] = invData;
			if (!removeQuestItems && entry->IsQuestObject()) {
				continue;
			}
			RE::ExtraDataList* a_extraList = new RE::ExtraDataList();
			if (keepOwnership) {
				a_extraList->SetOwner(entry->GetOwner());
			} else {
				a_extraList->SetOwner(to);
			}
			to->AddObjectToContainer(a_object, a_extraList, count, from);
		}
	}

	void Disintegrate(Actor* actor) {
		actor->criticalStage.set(ACTOR_CRITICAL_STAGE::kDisintegrateEnd);
		// CallFunctionOn(actor, "actor", "SetCriticalStage", 4);
		actor->Disable();
	}

	void UnDisintegrate(Actor* actor) {
		actor->criticalStage.reset(ACTOR_CRITICAL_STAGE::kDisintegrateEnd);
		// CallFunctionOn(actor, "Actor", "SetCriticalStage", 0);
		// actor->Enable();
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
		//CallFunctionOn(source, "ObjectReference", "PushActorAway", receiver, afKnockBackForce);
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
	void ApplyHavokImpulse(Actor* target, float afX, float afY, float afZ, float afMagnitude) {
		NiPoint3 direction = NiPoint3(afX, afY, afZ);
		NiPoint3 niImpulse  = direction * afMagnitude/direction.Length();
		hkVector4 impulse = hkVector4(niImpulse.x, niImpulse.y, niImpulse.z, 0.0);

		auto rbs = GetActorRB(target);
		for (auto rb: rbs) {
			auto& motion = rb->motion;
			motion.ApplyLinearImpulse(impulse);
		}
	}

	bool IsDragon(Actor* actor) {
		return (
			std::string(actor->GetDisplayFullName()).find("ragon") != std::string::npos
			|| Runtime::IsRace(actor, "dragonRace")
			);
	}

	bool IsProne(Actor* actor) {
		return actor!= nullptr && actor->formID == 0x14 && Runtime::GetBool("ProneEnabled") && actor->IsSneaking();
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
}
