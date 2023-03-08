#include "utils/actorUtils.hpp"
#include "utils/findActor.hpp"
#include "utils/papyrusUtils.hpp"
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

	bool IsDragon(Actor* actor) {
		return (
			std::string(actor->GetDisplayFullName()).find("ragon") != std::string::npos
			|| std::string(actor->GetDisplayFullName()).find("Dragon") != std::string::npos
			|| std::string(actor->GetDisplayFullName()).find("dragon") != std::string::npos
			|| Runtime::IsRace(actor, "dragonRace")
			);
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

	void ApplyShake(Actor* caster, Actor* receiver, float modifier) {
		auto player = PlayerCharacter::GetSingleton();
		float distance = get_distance_to_camera(caster);
		if (caster != player) {
			distance = get_distance_to_actor(caster, receiver);
		}
		float sourcesize = get_target_scale(caster);
		float receiversize = get_target_scale(receiver);
		float sizedifference = clamp(0.0, 10.0, sourcesize/receiversize);
		if (caster == receiver) {
			sizedifference = clamp(0.0, 10.0, sourcesize);
		}
		float falloff = 450 * (sourcesize * 0.25 + 0.75) * (sizedifference * 0.25 + 0.75);
		float power = (0.425 * ShakeStrength(caster));
		float duration = 0.25 * (1 + (sizedifference * 0.25));
		if (distance < falloff) {
			float intensity = ((falloff/distance) / 8);
			intensity = intensity*power;
			duration = duration * intensity;

			if (intensity <= 0) {
				intensity = 0;
			}
			if (power >= 12.6) {
				power = 12.6;
			}
			if (duration > 1.2) {
				duration = 1.2;
			}
			log::info("Shake, Source: {}, Receiver: {}, Intensity: {}, distance: {}, Falloff: {}", caster->GetDisplayFullName(), receiver->GetDisplayFullName(), intensity, distance, falloff);

			if (receiver == player) {
				shake_controller(intensity*modifier, intensity*modifier, duration);
				shake_camera(receiver, intensity*modifier, duration);
			}
		}
	}

	void ApplyShakeAtNode(Actor* caster, Actor* receiver, float modifier, const std::string_view& node) {
		auto player = PlayerCharacter::GetSingleton();
		float distance = get_distance_to_camera(caster);
		auto bone = find_node(caster, node);
		if (bone) {
			NiAVObject* attach = bone;
			if (attach) {
				distance = get_distance_to_camera(attach);
				log::info("Distance True");
			}
		}
		float sourcesize = get_target_scale(caster);
		float receiversize = get_target_scale(receiver);
		float sizedifference = clamp(0.0, 10.0, sourcesize/receiversize);
		if (caster == receiver) {
			sizedifference = clamp(0.0, 10.0, sourcesize);
		}
		float falloff = 450 * (sourcesize * 0.25 + 0.75) * (sizedifference * 0.25 + 0.75);
		float power = (0.425 * ShakeStrength(caster));
		float duration = 0.25 * (1 + (sizedifference * 0.25));
		if (distance < falloff) {
			float intensity = ((falloff/distance) / 8);
			intensity = intensity*power;
			duration = duration * intensity;

			if (intensity <= 0) {
				intensity = 0;
			}
			if (power >= 12.6) {
				power = 12.6;
			}
			if (duration > 1.2) {
				duration = 1.2;
			}
			log::info("Shake, Source: {}, Receiver: {}, Intensity: {}, distance: {}, Falloff: {}", caster->GetDisplayFullName(), receiver->GetDisplayFullName(), intensity, distance, falloff);

			if (receiver->formID == 0x14) {
				log::info("Playing Sound");
				shake_controller(intensity*modifier, intensity*modifier, duration);
				shake_camera(receiver, intensity*modifier, duration);
			}
		}
	}
}
