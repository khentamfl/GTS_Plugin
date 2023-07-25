#include "managers/animation/AnimationManager.hpp"
#include "managers/damage/AccurateDamage.hpp"
#include "managers/GtsSizeManager.hpp"
#include "magic/effects/common.hpp"
#include "utils/papyrusUtils.hpp"
#include "managers/explosion.hpp"
#include "managers/highheel.hpp"
#include "managers/footstep.hpp"
#include "utils/actorUtils.hpp"
#include "utils/findActor.hpp"
#include "data/persistent.hpp"
#include "data/transient.hpp"
#include "data/runtime.hpp"
#include "spring.hpp"
#include "scale/scale.hpp"
#include "colliders/RE.hpp"
#include "colliders/actor.hpp"
#include "profiler.hpp"
#include "timer.hpp"
#include "node.hpp"
#include "utils/av.hpp"
#include "colliders/RE.hpp"
#include "UI/DebugAPI.hpp"
#include <vector>
#include <string>

using namespace RE;
using namespace Gts;

namespace {
	float ShakeStrength(Actor* Source) {
		float Size = get_visual_scale(Source);
		float k = 0.065;
		float n = 1.0;
		float s = 1.12;
		float Result = 1.0/(pow(1.0+pow(k*(Size),n*s),1.0/s));
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

  struct SpringGrowData {
    Spring amount = Spring(0.0, 1.0);
    float addedSoFar = 0.0;
    ActorHandle actor;

    SpringGrowData(Actor* actor, float amountToAdd, float halfLife): actor(actor->CreateRefHandle()) {
      amount.value = 0.0;
      amount.target = amountToAdd;
      amount.halflife = halfLife;
    }
  };

  struct SpringShrinkData {
    Spring amount = Spring(0.0, 1.0);
    float addedSoFar = 0.0;
    ActorHandle actor;

    SpringShrinkData(Actor* actor, float amountToAdd, float halfLife): actor(actor->CreateRefHandle()) {
      amount.value = 0.0;
      amount.target = amountToAdd;
      amount.halflife = halfLife;
    }
  };
}

RE::ExtraDataList::~ExtraDataList() {
}

namespace Gts {
	Actor* GetActorPtr(Actor* actor) {
		return actor;
	}

	Actor* GetActorPtr(Actor& actor) {
		return &actor;
	}

	Actor* GetActorPtr(ActorHandle& actor) {
		if (!actor) {
			return nullptr;
		}
		return actor.get().get();
	}
	Actor* GetActorPtr(const ActorHandle& actor) {
		if (!actor) {
			return nullptr;
		}
		return actor.get().get();
	}
	Actor* GetActorPtr(FormID formId) {
		Actor* actor = TESForm::LookupByID<Actor>(formId);
		if (!actor) {
			return nullptr;
		}
		return actor;
	}

	float GetLaunchPower(float sizeRatio) {
		// https://www.desmos.com/calculator/wh0vwgljfl
		SoftPotential launch {
				.k = 1.42, 
				.n = 0.78, 
				.s = 0.6, 
				.a = 0.8, 
			};
		float power = soft_power(sizeRatio, launch);
		return power;
	}

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

	void ForceRagdoll(Actor* actor, bool forceOn) {
		if (!actor) {
			return;
		}
		auto charCont = actor->GetCharController();
		if (!charCont) {
			return;
		}
		BSAnimationGraphManagerPtr animGraphManager;
		if (actor->GetAnimationGraphManager(animGraphManager)) {
			for (auto& graph : animGraphManager->graphs) {
				if (graph) {
					if (graph->HasRagdoll()) {
						if (forceOn) {
							graph->AddRagdollToWorld();
							charCont->flags.set(CHARACTER_FLAGS::kFollowRagdoll);
						} else {
							graph->RemoveRagdollFromWorld();
							charCont->flags.reset(CHARACTER_FLAGS::kFollowRagdoll);
						}
					}
				}
			}
		}
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
		// CallFunctionOn(source, "ObjectReference", "PushActorAway", receiver, afKnockBackForce);

		if (source) {
			auto ai = receiver->GetActorRuntimeData().currentProcess;
			if (ai) {
				if (ai->InHighProcess()) {
				if (receiver->Is3DLoaded()) {
					if (source->Is3DLoaded()) {
					NiPoint3 direction = receiver->GetPosition() - source->GetPosition();
					direction = direction / direction.Length();

					typedef void(*DefPushActorAway)(AIProcess *ai, Actor* actor, NiPoint3& direction, float force);
					REL::Relocation<DefPushActorAway> RealPushActorAway{ RELOCATION_ID(38858, 39895) };
					RealPushActorAway(ai, receiver, direction, afKnockBackForce);
						}
					}
				}
			}
		}
	}

	void PushActorAway(TESObjectREFR* source, Actor* receiver, NiPoint3 direction, float force) {
		if (receiver->IsDead()) {
			return;
		}
		// CallFunctionOn(source, "ObjectReference", "PushActorAway", receiver, afKnockBackForce);

		if (source) {
			auto ai = receiver->GetActorRuntimeData().currentProcess;
			if (ai) {
				if (ai->InHighProcess()) {
				if (receiver->Is3DLoaded()) {
					if (source->Is3DLoaded()) {
					log::info("Direction: {}, force {}", Vector2Str(direction), force);

					typedef void(*DefPushActorAway)(AIProcess *ai, Actor* actor, NiPoint3& direction, float force);
					REL::Relocation<DefPushActorAway> RealPushActorAway{ RELOCATION_ID(38858, 39895) };
					RealPushActorAway(ai, receiver, direction, force);
						}
					}
				}
			}
		}
	}

	void KnockAreaEffect(TESObjectREFR* source, float afMagnitude, float afRadius) {
		CallFunctionOn(source, "ObjectReference", "KnockAreaEffect", afMagnitude, afRadius);
	}
	void ApplyHavokImpulse(Actor* target, float afX, float afY, float afZ, float afMagnitude) {
		if (afZ <= 0) {
			return;
		}
		NiPoint3 direction = NiPoint3(afX, afY, afZ);
		NiPoint3 niImpulse = direction * afMagnitude/direction.Length();
		hkVector4 impulse = hkVector4(niImpulse.x, niImpulse.y, niImpulse.z, 0.0);
		auto rbs = GetActorRB(target);
		for (auto rb: rbs) {
			if (rb) {
				auto& motion = rb->motion;
				motion.ApplyLinearImpulse(impulse);
			}
		}
	}
	void ApplyHavokImpulse(TESObjectREFR* target, float afX, float afY, float afZ, float afMagnitude) {
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

	bool IsGrowthSpurtActive(Actor* actor) {
		if (!Runtime::HasPerkTeam(actor, "GrowthOfStrength")) {
			return false;
		}
		if (HasGrowthSpurt(actor)) {
			return true;
		}
		return false;
	}

	bool HasGrowthSpurt(Actor* actor) {
		bool Growth1 = Runtime::HasMagicEffect(actor, "explosiveGrowth1");
		bool Growth2 = Runtime::HasMagicEffect(actor, "explosiveGrowth2");
		bool Growth3 = Runtime::HasMagicEffect(actor, "explosiveGrowth3");
		if (Growth1 || Growth2 || Growth3) {
			return true;
		} else {
			return false;
		}
	}

	bool AllowStagger(Actor* giant, Actor* tiny) {
		if (Persistent::GetSingleton().allow_stagger == true) {
			//log::info("Allow_Stagger TRUE: {}, IsTeammate: {} {}", Persistent::GetSingleton().allow_stagger, tiny->GetDisplayFullName(), IsTeammate(tiny));
			return true; // Allow it
		} else if (Persistent::GetSingleton().allow_stagger == false && (giant->formID == 0x14 || IsTeammate(giant)) && (tiny->formID == 0x14 || IsTeammate(tiny))) {
			//log::info("Allow_Stagger FALSE: {}, IsTeammate: {} {}", Persistent::GetSingleton().allow_stagger, tiny->GetDisplayFullName(), IsTeammate(tiny));
			return false; // Protect
		}
		log::info("Stagger false");
		return true;
	}
    
	bool IsHuman(Actor* actor) { // Check if Actor is humanoid or not. Currently used for Hugs Animation
		bool vampire = Runtime::HasKeyword(actor, "VampireKeyword");
		bool dragon = Runtime::HasKeyword(actor, "DragonKeyword");
		bool animal = Runtime::HasKeyword(actor, "AnimalKeyword");
		bool dwemer = Runtime::HasKeyword(actor, "DwemerKeyword");
		bool undead = Runtime::HasKeyword(actor, "UndeadKeyword");
		bool creature = Runtime::HasKeyword(actor, "CreatureKeyword");
		log::info("{} is vamp: {}, drag: {}, anim: {}, dwem: {}, undead: {}, creat: {}", actor->GetDisplayFullName(), vampire, dragon, animal, dwemer, undead, creature);
		if (!dragon && !animal && !dwemer && !undead && !creature) {
			return true; // Detect non-vampire
		} if (!dragon && !animal && !dwemer && !creature && undead && vampire) {
			return true; // Detect Vampire
		} else {
			return false;
		}
		return false;
	}

	bool IsFemale(Actor* actor) {
		bool FemaleCheck = false;
		if (!FemaleCheck) {
			return true; // Always return true if we don't check for male/female
		}
		auto base = actor->GetActorBase();
		int sex = 0;
		if (base) {
			if (base->GetSex()) {
				sex = base->GetSex();
			}
		}
		log::info("Sex of {}: {}", actor->GetDisplayFullName(), sex);
		return sex > 0; // Else return sex value
	}

	bool IsDragon(Actor* actor) {
		if (Runtime::HasKeyword(actor, "DragonKeyword")) {
			return true;
		}
		if ( std::string(actor->GetDisplayFullName()).find("ragon") != std::string::npos
		     || Runtime::IsRace(actor, "dragonRace")) {
			return true;
		} else {
			return false;
		}
	}

	bool IsLiving(Actor* actor) {
		bool IsDraugr = Runtime::HasKeyword(actor, "UndeadKeyword");
		bool IsDwemer = Runtime::HasKeyword(actor, "DwemerKeyword");
		bool IsVampire = Runtime::HasKeyword(actor, "VampireKeyword");
		if (IsVampire) {
			log::info("{} is Vampire", actor->GetDisplayFullName());
			return true;
		}
		if (IsDraugr || IsDwemer) {
			log::info("{} is not living", actor->GetDisplayFullName());
			return false;
		} else {
			return true;
		}
		return true;
	}

	bool IsEquipBusy(Actor* actor) {
		auto profiler = Profilers::Profile("ActorUtils: IsEquipBusy");
		int State;
		actor->GetGraphVariableInt("currentDefaultState", State);
		if (State >= 10 && State <= 20) {
			return true;
		}
		return false;
	}

	bool IsCrawling(Actor* actor) {
		auto profiler = Profilers::Profile("ActorUtils: IsCrawling");
		bool prone;
		actor->GetGraphVariableBool("GTS_IsCrawling", prone);
		return actor!= nullptr && actor->formID == 0x14 && actor->IsSneaking() && prone;
	}

	bool IsJumping(Actor* actor) {
		auto profiler = Profilers::Profile("ActorUtils: IsJumping");
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

	float GetHighHeelsBonusDamage(Actor* actor) {
		auto profiler = Profilers::Profile("ActorUtils: GetHHBonusDamage");
		if (Runtime::HasPerkTeam(actor, "hhBonus")) {
			return HighHeelManager::GetBaseHHOffset(actor).Length()/100;
		}
		return 1.0;
	}

	float get_distance_to_actor(Actor* receiver, Actor* target) {
		auto profiler = Profilers::Profile("ActorUtils: GetDistanceToActor");
		if (target) {
			auto point_a = receiver->GetPosition();
			auto point_b = target->GetPosition();
			auto delta = point_a - point_b;
			return delta.Length();
		}
		return 3.4028237E38; // Max float
	}

	void ApplyShake(Actor* caster, float modifier, float radius) {
		if (caster) {
			auto position = caster->GetPosition();
			ApplyShakeAtPoint(caster, modifier, position, radius);
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
		float sizedifference = (sourcesize/receiversize) * ShakeStrength(caster);
		if (caster->formID != 0x14) {
			log::info("Shake Strength of :{}", caster->GetDisplayFullName(), ShakeStrength(caster));
		}
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
			float intensity = (sizedifference * 18.8) / distance;
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
	bool LessGore() {
		log::info("Less gore is {}", Persistent::GetSingleton().less_gore);
		return Persistent::GetSingleton().less_gore;
	}

	void SetBeingHeld(Actor* tiny, bool decide) {
		auto transient = Transient::GetSingleton().GetData(tiny);
		if (transient) {
			transient->being_held = decide;
		}
	}

	bool IsBeingHeld(Actor* tiny) {
		auto transient = Transient::GetSingleton().GetData(tiny);
		if (transient) {
			return transient->being_held;
		}
		return false;
	}

	bool IsBetweenBreasts(Actor* actor) {
		auto transient = Transient::GetSingleton().GetData(actor);
		if (transient) {
			return transient->between_breasts;
		}
		return false;
	}

	void SetBetweenBreasts(Actor* actor, bool decide) {
		auto transient = Transient::GetSingleton().GetData(actor);
		if (transient) {
			transient->between_breasts = decide;
		}
	}

	bool IsTransferingTiny(Actor* actor) { // Reports 'Do we have someone grabed?'
		int grabbed;
		actor->GetGraphVariableInt("GTS_GrabbedTiny", grabbed);
		return grabbed > 0;
	}

	bool IsUsingThighAnimations(Actor* actor) { // Do we currently use Thigh Crush / Thigh Sandwich?
		int sitting;
		actor->GetGraphVariableInt("GTS_Sitting", sitting);
		return sitting > 0;
	}

	void SetBeingEaten(Actor* tiny, bool decide) {
		auto transient = Transient::GetSingleton().GetData(tiny);
		if (transient) {
			transient->about_to_be_eaten = decide;
		}
	}

	bool IsBeingEaten(Actor* tiny) {
		auto transient = Transient::GetSingleton().GetData(tiny);
		if (transient) {
			return transient->about_to_be_eaten;
		}
		return false;
	}

	bool IsGtsBusy(Actor* actor) {
		auto profiler = Profilers::Profile("ActorUtils: IsGtsBusy");
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

	void AddCalamityPerk() {
		auto progressionQuest = Runtime::GetQuest("MainQuest");
		if (progressionQuest) {
			CallFunctionOn(progressionQuest, "gtsProgressionQuest", "AddCalamityPerk");
		}
	}

	void AddPerkPoints(float level) {
		auto GtsSkillPerkPoints = Runtime::GetGlobal("GtsSkillPerkPoints");
		if (!GtsSkillPerkPoints) {
			return;
		}
		if (int(level) % 5 == 0) {
			Notify("You've learned a bonus perk point");
			GtsSkillPerkPoints->value += 1.0;
		} if (level == 20 || level == 40) {
			GtsSkillPerkPoints->value += 2.0;
		} else if (level == 60 || level == 80) {
			GtsSkillPerkPoints->value += 3.0;
		} else if (level == 100) {
			GtsSkillPerkPoints->value += 4.0;
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

	void SpawnDustParticle(Actor* giant, Actor* tiny, std::string_view node, float size) {
		auto result = find_node(giant, node);
		if (result) {
			BGSExplosion* base_explosion = Runtime::GetExplosion("draugrexplosion");
			if (base_explosion) {
				NiPointer<TESObjectREFR> instance_ptr = giant->PlaceObjectAtMe(base_explosion, false);
				if (!instance_ptr) {
					return;
				}
				TESObjectREFR* instance = instance_ptr.get();
				if (!instance) {
					return;
				}
				Explosion* explosion = instance->AsExplosion();
				if (!explosion) {
					return;
				}
				explosion->SetPosition(result->world.translate);
				explosion->GetExplosionRuntimeData().radius *= 3 * get_visual_scale(tiny) * size;
				explosion->GetExplosionRuntimeData().imodRadius *= 3 * get_visual_scale(tiny) * size;
				explosion->GetExplosionRuntimeData().unkB8 = nullptr;
				explosion->GetExplosionRuntimeData().negativeVelocity *= 0.0;
				explosion->GetExplosionRuntimeData().unk11C *= 0.0;
			}
		}
	}

	void ShutUp(Actor* actor) { // Disallow them to "So anyway i've been fishing today and my dog died" while we do something to them
		if (!actor) {
			return;
		}
		auto ai = actor->GetActorRuntimeData().currentProcess;
		if (ai) {
			if (ai->high) {
				float Greeting = ai->high->greetingTimer;
				ai->high->greetingTimer = 5;
			}
			//log::info("Greeting timer of {} is {}", actor->GetDisplayFullName(), Greeting);
		}
	}

	void StaggerOr(Actor* giant, Actor* tiny, float power, float afX, float afY, float afZ, float afMagnitude) {
		if (tiny->IsDead()) {
			return;
		}
		if (IsBeingHeld(tiny)) {
			return;
		}
		if (!AllowStagger(giant, tiny)) {
			return;
		}
		float giantSize = get_visual_scale(giant);
		float tinySize = get_visual_scale(tiny);
		if (HasSMT(giant)) {
			giantSize *= 4.0;
		}
		float sizedifference = giantSize/tinySize;
		int ragdollchance = rand() % 30 + 1.0;
		if (sizedifference >= 3.0) {
			//ForceRagdoll(tiny, true);
			PushActorAway(giant, tiny, power/50); // Always push
			//ApplyHavokImpulse(tiny, afX, afY, afZ, afMagnitude);
			return;
		}
		if (power >= 0.50 || ragdollchance >= 7.0 * sizedifference && (sizedifference >= 1.25 && sizedifference < 3.0)) {
			tiny->SetGraphVariableFloat("staggerMagnitude", 100.00f); // Stagger actor
			tiny->NotifyAnimationGraph("staggerStart");
			return;
		} else if (ragdollchance < 7.0 * sizedifference) {
			// ForceRagdoll(tiny, true);
			PushActorAway(giant, tiny, power/50); // Push instead
			ApplyHavokImpulse(tiny, afX, afY, afZ, afMagnitude);
			return;
		}
	}

	void DoDamageEffect(Actor* giant, float damage, float radius, int random, float bonedamage, FootEvent kind, float crushmult) {
		radius *= 1.0 + (GetHighHeelsBonusDamage(giant) * 2.5);
		if (kind == FootEvent::Left) {
			AccurateDamage::GetSingleton().DoAccurateCollisionLeft(giant, (45.0 * damage), radius, random, bonedamage, crushmult);
		} if (kind == FootEvent::Right) {
			AccurateDamage::GetSingleton().DoAccurateCollisionRight(giant, (45.0 * damage), radius, random, bonedamage, crushmult);
			//                                                                                         ^        ^           ^ - - - - Normal Crush 
			//                                                               Chance to trigger bone crush   Damage of            Threshold multiplication
			//                                                                                             Bone Crush
		}
	}

	void SizeStealExplosion(Actor* giant, float radius, NiAVObject* node, float shrink, bool WasHit) {
		if (!node) {
			return;
		} if (!giant) {
			return;
		}
		float giantScale = get_visual_scale(giant);
		float gigantism = 1.0 + SizeManager::GetSingleton().GetEnchantmentBonus(giant)*0.01;
		bool DarkArts1 =  Runtime::HasPerk(giant, "DarkArts_Aug");
		bool DarkArts2 = Runtime::HasPerk(giant, "DarkArts_Aug2");
		bool DarkArts_Max = Runtime::HasPerk(giant, "DarkArts_Max");
		float explosion = 0.7;

		if (DarkArts1) {
			radius *= 1.33;
			shrink *= 1.33;
			explosion = 1.0;
		} if (WasHit) {
			explosion = 2.0;
		}

		float SCALE_RATIO = 0.75;

		shake_camera(giant, 0.70, 0.55);

		NiPoint3 NodePosition = node->world.translate;

		float maxDistance = radius * giantScale * gigantism;
		// Make a list of points to check
		std::vector<NiPoint3> points = {
			NiPoint3(0.0, 0.0, 0.0), // The standard position
		};
		std::vector<NiPoint3> Points = {};

		Runtime::CreateExplosionAtPos(giant, NodePosition, giantScale * explosion, "SizeStealExplosion");

		for (NiPoint3 point: points) {
			Points.push_back(NodePosition);
		}
		if (Runtime::GetBool("EnableDebugOverlay") && (giant->formID == 0x14 || giant->IsPlayerTeammate() || Runtime::InFaction(giant, "FollowerFaction"))) {
			for (auto point: Points) {
				DebugAPI::DrawSphere(glm::vec3(point.x, point.y, point.z), maxDistance, 600, {0.0, 1.0, 0.0, 1.0});
			}
		}

		NiPoint3 giantLocation = giant->GetPosition();

		for (auto otherActor: find_actors()) {
			if (otherActor != giant) { 
				float tinyScale = get_visual_scale(otherActor);
				if (giantScale / tinyScale > SCALE_RATIO) {
					NiPoint3 actorLocation = otherActor->GetPosition();
					if ((actorLocation-giantLocation).Length() <= maxDistance * 2.0) {
						int nodeCollisions = 0;
						float force = 0.0;
						auto model = otherActor->GetCurrent3D();
						if (model) {
							for (auto point: Points) {
								VisitNodes(model, [&nodeCollisions, &force, point, maxDistance](NiAVObject& a_obj) {
									float distance = (point - a_obj.world.translate).Length();
									if (distance < maxDistance) {
										nodeCollisions += 1;
										force = 1.0 - distance / maxDistance;
									}
									return true;
								});
							}
						}
						if (nodeCollisions > 1) {
							float sizedifference = giantScale/get_visual_scale(otherActor);
							float shrinkpower = -(shrink * 0.5) * (1.0 + GetGtsSkillLevel() * 0.005) * CalcEffeciency(giant, otherActor);
							if (DarkArts2 && (IsGrowthSpurtActive(giant) || HasSMT(giant))) {
								shrinkpower *= 2.0;
							}
							if (sizedifference <= 4.0) {
								StaggerActor(otherActor);
							} else {
								PushActorAway(giant, otherActor, 1.0 * GetLaunchPower(sizedifference));
							}
								
							mod_target_scale(otherActor, shrinkpower * gigantism);

							std::string taskname = std::format("ShrinkOtherCheck_{}", otherActor->formID);
							const float DURATION = 2.5;

							ActorHandle tinyHandle = otherActor->CreateRefHandle();
							ActorHandle giantHandle = giant->CreateRefHandle();
							
							TaskManager::RunFor(taskname, DURATION, [=](auto& progressData){
								auto GTS = giantHandle.get().get();
								auto TINY = tinyHandle.get().get();
								if (!GTS) {
									return false;
								} if (!TINY) {
									return false;
								}
								if (get_visual_scale(TINY) <= 0.14) {
									set_target_scale(TINY, 0.14);
									return false;
								}
								return true; // Everything is fine, continue checking
							});
						}
					}
				}
			}
		}
	}
	

	bool HasSMT(Actor* giant) {
		if (Runtime::HasMagicEffect(giant, "SmallMassiveThreat")) {
			return true;
		}
		return false;
	}

	void TiredSound(Actor* player, std::string_view message) {
		static Timer Cooldown = Timer(1.2);
		if (Cooldown.ShouldRun()) {
			Runtime::PlaySound("VoreSound_Fail", player, 0.7, 0.0);
			Notify(message);
		}
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
			return 1.5;
		} else if (giant->AsActorState()->IsSneaking()) {
			return 0.6;
		} else {
			return 1.0;
		}
	}

	float GetGtsSkillLevel() {
		auto GtsSkillLevel = Runtime::GetGlobal("GtsSkillLevel");
		return GtsSkillLevel->value;
	}

	float GetXpBonus() {
		float xp = Persistent::GetSingleton().experience_mult;
		//log::info("XP is: {}", xp);
		return xp;
	}

	void AddSMTDuration(Actor* actor, float duration) {
		if (!HasSMT(actor)) {
			return;
		}
		if (Runtime::HasPerk(actor, "EternalCalamity")) {
			auto transient = Transient::GetSingleton().GetData(actor);
			if (transient) {
				transient->SMT_Bonus_Duration += duration;
				log::info("SMT Duration Added: {}", duration);
			}
		}
	}

	void AddSMTPenalty(Actor* actor, float penalty) {
		auto transient = Transient::GetSingleton().GetData(actor);
		if (transient) {
			transient->SMT_Penalty_Duration += penalty;
			log::info("SMT Penalty Added: {}", penalty);
		}
	}

	void PrintDeathSource(Actor* giant, Actor* tiny, std::string_view cause) {
		int random = rand()% 8;
		float sizedifference = get_visual_scale(giant)/get_visual_scale(tiny);
		if (cause == "Crushed") { // Default crush by the feet
			if (random < 2) {
				Cprint("{} became a bloody stain under {} foot.", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random == 2) {
				Cprint("{} was crushed by the feet of {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random == 3) {
				Cprint("Feet of {} crushed {} into nothing", giant->GetDisplayFullName(), tiny->GetDisplayFullName());
			} else if (random == 4) {
				Cprint("{} stepped on {} too hard", giant->GetDisplayFullName(), tiny->GetDisplayFullName());
			} else if (random == 5) {
				Cprint("{} got crushed by {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random == 6) {
				Cprint("{} stepped on {} too hard", giant->GetDisplayFullName(), tiny->GetDisplayFullName());
			} else if (random >= 7) {
				Cprint("{} relentlessly crushed {}", giant->GetDisplayFullName(), tiny->GetDisplayFullName());
			}
			return;
		} else if (cause == "HandCrushed") { // when Grab -> Crush happens
			if (random == 1) {
				Cprint("{} life was squeezed out in {} grip", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random == 2) {
				Cprint("{} Squeezed her palm, unexpectedly crushing {}", giant->GetDisplayFullName(), tiny->GetDisplayFullName());
			} else if (random == 3) {
				Cprint("{} was transformed into bloody mist by the tight grip of {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random == 4) {
				Cprint("{} has been crushed inside the hand of {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random >= 6) {
				Cprint("{} applied too much pressure to her hand, crushing {}", giant->GetDisplayFullName(), tiny->GetDisplayFullName());
			} else if (random >= 7) {
				Cprint("{} was turned into pulp by the palm of {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			}
			return;
		} else if (cause == "Shrinked") { // Shrink to nothing
			if (random <= 2) {
				Cprint("{} greedily absorbed {}", giant->GetDisplayFullName(), tiny->GetDisplayFullName());
			} else if (random == 3) {
				Cprint("{} completely absorbed {}", giant->GetDisplayFullName(), tiny->GetDisplayFullName());
			} else if (random == 4) {
				Cprint("{} was absorbed by {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random == 5) {
				Cprint("{} was shrinkned to nothing by {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random == 6) {
				Cprint("{} size was completely drained by {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random >= 7) {
				Cprint("{} stole all the size from {}, exploding {}", giant->GetDisplayFullName(), tiny->GetDisplayFullName());
			}
			return;
		} else if (cause == "Vored") {
			///nothing for now
			return;
		} else if (cause == "ThighCrushed") { // During thigh crush
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
		} else if (cause == "ThighSandwiched") { // Self explanatory
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
		} else if (cause == "Overkill") {  // When we hit actor with too much weapon damage while being huge
			if (random == 1) {
				Cprint("{} body exploded because of massive size difference with {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random == 2) {
				Cprint("{} hit {} with so much force that {} exploded into bloody mist", giant->GetDisplayFullName(), tiny->GetDisplayFullName(), tiny->GetDisplayFullName());
			} else if (random == 3) {
				Cprint("{} was pulverized into nothing by {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random == 4) {
				Cprint("All that's left from {} after being hit bit {} is a bloody mist", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random >= 6) {
				Cprint("{} couldn't handle enormous hit from {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random >= 7) {
				Cprint("{} put so much force into attack that {} turned into red mist", giant->GetDisplayFullName(), tiny->GetDisplayFullName());
			}
			return;
		} else if (cause == "HitSteal") { // Hit Growth perk
			if (random <= 2) {
				Cprint("{} body exploded after trying to hit {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random == 3) {
				Cprint("Protective magic of {} made {} absorb {}", giant->GetDisplayFullName(), giant->GetDisplayFullName(), tiny->GetDisplayFullName());
			} else if (random > 3) {
				Cprint("{} Tried to kill {}, but ended up being absorbed by the size magic of {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName(), giant->GetDisplayFullName());
			}
			return;
		} else if (cause == "Explode") { // Poison Of Shrinking
			if (random <= 2) {
				Cprint("{} exploded into bloody dust", tiny->GetDisplayFullName());
			} else if (random == 3) {
				Cprint("{} suddenly exploded", tiny->GetDisplayFullName());
			} else if (random > 3) {
				Cprint("{} was turned into nothing", tiny->GetDisplayFullName());
			}
			return;
		} else if (cause == "BlockDamage") { // WHen tiny in hand receives too much damage
			if (random == 1) {
				Cprint("{} received too much damage and was automatically crushed in the hands of {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random == 2) {
				Cprint("{} was crushed from receiving too much damage {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random == 3) {
				Cprint("{} stopped to be useful, so he was turned into bloody mist in the hands of {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random == 4) {
				Cprint("{} took extreme amounts of damage and exploded inside the hands of {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random >= 6) {
				Cprint("{} took a little more damage than intended, so her fingers ended up squeezing {} into nothing", giant->GetDisplayFullName(), tiny->GetDisplayFullName());
			} else if (random >= 7) {
				Cprint("{} blocked too much damage and was squeezed into bloody stain by {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			}
			return;
		} else if (cause == "Breasts") { // Someone died between breasts
			if (random == 1) {
				Cprint("{} was weakened and got accidentally crushed by {} breasts", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random == 2) {
				Cprint("{} got unintentionally crushed by the breasts of {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random == 3) {
				Cprint("{} left this world by being crushed between the cleavage of {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random == 4) {
				Cprint("Breasts of {} squeezed all life out of {}", giant->GetDisplayFullName(), tiny->GetDisplayFullName());
			} else if (random >= 6) {
				Cprint("{} took some damage and ended up crushing {} between her breasts", giant->GetDisplayFullName(), tiny->GetDisplayFullName());
			} else if (random >= 7) {
				Cprint("{} got smothered by soft breasts of {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			}
			return;
		} else if (cause == "Grinded") { // Grinded by the foot. Currently doesn't exist. It is planned to add it.
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
		} else if (cause == "Melted") { // Melted by tongue. Currently doesn't exist. Possible Vore variation with melting actors with tongue instead.
			if (random < 2) {
				Cprint("{} was melted by the tongue of {}.", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random == 2) {
				Cprint("{} got absorbed by the tongue of {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random == 3 || random == 4) {
				Cprint("Hot tongue of {} melted {} like a candy", giant->GetDisplayFullName(), tiny->GetDisplayFullName());
			} else if (random == 5 || random == 6) {
				Cprint("{} was (un)forunate enough to be melted by the tongue of {} ", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random >= 7) {
				Cprint("Tongue of {} sucked all life out of {}", giant->GetDisplayFullName(), tiny->GetDisplayFullName());
			}
			return;
		} else if (cause == "Butt") { // Butt Crushed. Currently doesn't exist. It is planned though.
			if (random < 2) {
				Cprint("{} got crushed by the butt of {}.", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random == 2) {
				Cprint("{} was overwhelmed by the booty of {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random == 3) {
				Cprint("Bootie of {} completely pulverized {}", giant->GetDisplayFullName(), tiny->GetDisplayFullName());
			} else if (random == 4) {
				Cprint("Booty of {} completely pulverized {}", giant->GetDisplayFullName(), tiny->GetDisplayFullName());
			} else if (random == 5) {
				Cprint("{} has been squashed by butt attack of {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random == 6) {
				Cprint("{} cheeks dropped on {}, turning {} into red paste", giant->GetDisplayFullName(), tiny->GetDisplayFullName());
			}else if (random >= 7) {
				Cprint("{} relentlessly crushed {} with butt attack", giant->GetDisplayFullName(), tiny->GetDisplayFullName());
			}
			return;
		} else if (cause == "Hugs") { // For Hug Crush
			if (random < 2) {
				Cprint("{} was hugged to death by {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random == 2) {
				Cprint("{} got too carried away hugging {}, crushing {} as a result", giant->GetDisplayFullName(), tiny->GetDisplayFullName());
			} else if (random == 3) {
				Cprint("{} applied too much force to the hugs, killing {}", giant->GetDisplayFullName(), tiny->GetDisplayFullName());
			} else if (random == 4) {
				Cprint("{} couldn't resist hug crushing {}", giant->GetDisplayFullName(), tiny->GetDisplayFullName());
			} else if (random == 5) {
				Cprint("{} failed to escape hugs of death with {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random == 6) {
				Cprint("{} got greedy and stole all size from {} through hugs", giant->GetDisplayFullName(), tiny->GetDisplayFullName());
			} else if (random >= 7) {
				Cprint("{} gently hug crushed {}", giant->GetDisplayFullName(), tiny->GetDisplayFullName());
			}
			return;
		} else if (cause == "Crawl") { // For being crushed while GTS crawls
			if (random < 2) {
				Cprint("{} was accidentally crushed by {} that was crawling around", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random == 2) {
				Cprint("{} was crawling around and ended up killing {}", giant->GetDisplayFullName(), tiny->GetDisplayFullName());
			} else if (random == 3) {
				Cprint("{} ended up being in a wrong place and in the wrong time", tiny->GetDisplayFullName());
			} else if (random == 4) {
				Cprint("{} couldn't survive being under crawling {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random == 5) {
				Cprint("For some reason something has crunched under {}", giant->GetDisplayFullName());
			} else if (random == 6) {
				Cprint("{} couldn't survive under {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random >= 7) {
				Cprint("{} didn't realize that it's extremely dangerous to be under {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			}
			return;
		}
	}

	void PrintSuffocate(Actor* pred, Actor* prey) {
		int random = rand() % 6;
		if (random <= 1) {
			Cprint("{} was slowly smothered between {} thighs", prey->GetDisplayFullName(), pred->GetDisplayFullName());
		} else if (random == 2) {
			Cprint("{} was suffocated by the thighs of {}", prey->GetDisplayFullName(), pred->GetDisplayFullName());
		} else if (random == 3) {
			Cprint("Thighs of {} suffocated {} to death", pred->GetDisplayFullName(), prey->GetDisplayFullName());
		} else if (random == 4) {
			Cprint("{} got smothered between the thighs of {}", prey->GetDisplayFullName(), pred->GetDisplayFullName());
		} else if (random >= 5) {
			Cprint("{} lost life to the thighs of {}", prey->GetDisplayFullName(), pred->GetDisplayFullName());
		}
	}

  	void ShrinkUntil(Actor* giant, Actor* tiny, float expected) {
		if (HasSMT(giant)) {
			float predscale = get_target_scale(giant);
			float preyscale = get_target_scale(tiny);
			float targetScale = predscale/expected;
			if (preyscale >= targetScale) { // Apply ONLY if target is bigger than requirement
				set_target_scale(tiny, targetScale);
				AddSMTPenalty(giant, 5.0);
				log::info("Shrink: {}, Old Scale: {}, New Scale: {}", tiny->GetDisplayFullName(), preyscale, get_target_scale(tiny));
			}
		}
	}

  void DisableCollisions(Actor* actor, TESObjectREFR* otherActor) {
    if (actor) {
      auto trans = Transient::GetSingleton().GetData(actor);
      if (trans) {
        trans->disable_collision_with = otherActor;
        log::info("Disable collision for: {}", actor->GetDisplayFullName());
        auto colliders = ActorCollisionData(actor);
        colliders.UpdateCollisionFilter();
        if (otherActor) {
          Actor* asOtherActor = skyrim_cast<Actor*>(otherActor);
          auto otherColliders = ActorCollisionData(asOtherActor);
          otherColliders.UpdateCollisionFilter();
        }
      }
    }
  }
  void EnableCollisions(Actor* actor) {
    if (actor) {
      auto trans = Transient::GetSingleton().GetData(actor);
      if (trans) {
        auto otherActor = trans->disable_collision_with;
        trans->disable_collision_with = nullptr;
        log::info("Enable collision for: {}", actor->GetDisplayFullName());
        auto colliders = ActorCollisionData(actor);
        colliders.UpdateCollisionFilter();
        if (otherActor) {
          Actor* asOtherActor = skyrim_cast<Actor*>(otherActor);
          auto otherColliders = ActorCollisionData(asOtherActor);
          otherColliders.UpdateCollisionFilter();
        }
      }
    }
  }

  void SpringGrow(Actor* actor, float amt, float halfLife, std::string_view naming) {
    if (!actor) {
      return;
    }

    auto growData = std::make_shared<SpringGrowData>(actor, amt, halfLife);
	std::string name = std::format("SpringGrow {}: {}", naming, actor->formID);
	const float DURATION = halfLife * 3.2;

    TaskManager::RunFor(DURATION,
		[ growData ](const auto& progressData) {
		float totalScaleToAdd = growData->amount.value;
		float prevScaleAdded = growData->addedSoFar;
		float deltaScale = totalScaleToAdd - prevScaleAdded;
		Actor* actor = growData->actor.get().get();

		if (actor) {
			float stamina = clamp(0.05, 1.0, GetStaminaPercentage(actor));
			DamageAV(actor, ActorValue::kStamina, 0.55 * (get_visual_scale(actor) * 0.5 + 0.5) * stamina * TimeScale());
			auto actorData = Persistent::GetSingleton().GetData(actor);
			if (actorData) {
				actorData->target_scale += deltaScale;
				actorData->visual_scale += deltaScale;
				growData->addedSoFar = totalScaleToAdd;
				}
			}
        return fabs(growData->amount.value - growData->amount.target) > 1e-4;
      }
    );
  }

  void SpringGrow_Free(Actor* actor, float amt, float halfLife, std::string_view naming) {
    if (!actor) {
      return;
    }

    auto growData = std::make_shared<SpringGrowData>(actor, amt, halfLife);
	std::string name = std::format("SpringGrow_Free {}: {}", naming, actor->formID);
	const float DURATION = halfLife * 3.2;

    TaskManager::RunFor(name, DURATION,
		[ growData ](const auto& progressData) {
		float totalScaleToAdd = growData->amount.value;
		float prevScaleAdded = growData->addedSoFar;
		float deltaScale = totalScaleToAdd - prevScaleAdded;
		Actor* actor = growData->actor.get().get();

		if (actor) {
			auto actorData = Persistent::GetSingleton().GetData(actor);
			if (actorData) {
				actorData->target_scale += deltaScale;
				actorData->visual_scale += deltaScale;
				growData->addedSoFar = totalScaleToAdd;
				}
			}
        return fabs(growData->amount.value - growData->amount.target) > 1e-4;
      }
    );
  }

  void SpringShrink(Actor* actor, float amt, float halfLife, std::string_view naming) {
    if (!actor) {
      return;
    }

    auto growData = std::make_shared<SpringShrinkData>(actor, amt, halfLife);
	std::string name = std::format("SpringShrink {}: {}", naming, actor->formID);
	const float DURATION = halfLife * 3.2;
    TaskManager::RunFor(DURATION,
	[ growData ](const auto& progressData) {
		float totalScaleToAdd = growData->amount.value;
		float prevScaleAdded = growData->addedSoFar;
		float deltaScale = totalScaleToAdd - prevScaleAdded;
		Actor* actor = growData->actor.get().get();

		if (actor) {
			float stamina = clamp(0.05, 1.0, GetStaminaPercentage(actor));
			DamageAV(actor, ActorValue::kStamina, 0.35 * (get_visual_scale(actor) * 0.5 + 0.5) * stamina * TimeScale());	
          	auto actorData = Persistent::GetSingleton().GetData(actor);
			if (actorData) {
				actorData->target_scale += deltaScale;
				actorData->visual_scale += deltaScale;
            	growData->addedSoFar = totalScaleToAdd;
        	}
	    }

        return fabs(growData->amount.value - growData->amount.target) > 1e-4;
      }
    );
  }
}
