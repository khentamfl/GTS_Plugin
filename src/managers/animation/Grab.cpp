#include "managers/animation/AnimationManager.hpp"
#include "managers/emotions/EmotionManager.hpp"
#include "managers/ShrinkToNothingManager.hpp"
#include "managers/damage/SizeHitEffects.hpp"
#include "managers/animation/Grab.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/ai/aifunctions.hpp"
#include "managers/CrushManager.hpp"
#include "managers/InputManager.hpp"
#include "magic/effects/common.hpp"
#include "utils/actorUtils.hpp"
#include "data/persistent.hpp"
#include "managers/tremor.hpp"
#include "managers/Rumble.hpp"
#include "data/transient.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "data/time.hpp"
#include "events.hpp"
#include "timer.hpp"
#include "node.hpp"

#include <random>

using namespace RE;
using namespace REL;
using namespace Gts;
using namespace std;

///GTS_GrabbedTiny MUST BE 1 when we have someone in hands

/*Event used in the behaviours to transition between most behaviour states
Grab Events
	GTSBEH_GrabStart
	GTSBEH_GrabVore
	GTSBEH_GrabAttack
	GTSBEH_GrabThrow
	GTSBEH_GrabRelease
More Grab things we don't need to do anything with in the DLL
	GTSBeh_MT
	GTSBeh_1hm
	GTSBeh_Mag
	GTSBeh_Set
	GTSBeh_GrabVore_LegTrans
Used to leave the grab state
	GTSBeh_GrabExit
Grab Event to go back to vanilla
	GTSBEH_AbortGrab
*/


namespace {
	bool Escaped(Actor* giant, Actor* tiny, float strength) {
		float tiny_chance = ((rand() % 100000) / 100000.0f) * get_visual_scale(tiny);
		float giant_chance = ((rand() % 100000) / 100000.0f) * strength * get_visual_scale(giant);
		return (tiny_chance > giant_chance);
	}

	void ManageCamera(Actor* giant, bool enable, float type) {
		auto& sizemanager = SizeManager::GetSingleton();
		sizemanager.SetActionBool(giant, enable, type);
	}

	void AdjustFacialExpression(Actor* giant, int ph, float power, std::string_view type) {
		auto& Emotions = EmotionManager::GetSingleton().GetGiant(giant);
		float AnimSpeed = AnimationManager::GetSingleton().GetAnimSpeed(giant);

		if (type == "phenome") {
			Emotions.OverridePhenome(ph, 0.0, 0.08/AnimSpeed, power);
		}
		if (type == "expression") {
			auto fgen = giant->GetFaceGenAnimationData();
			if (fgen) {
				fgen->exprOverride = false;
				fgen->SetExpressionOverride(ph, power);
				fgen->expressionKeyFrame.SetValue(ph, power); // Expression doesn't need Spring since it is already smooth by default
				fgen->exprOverride = true;
			}
		}
		if (type == "modifier") {
			Emotions.OverrideModifier(ph, 0.0, 0.25/AnimSpeed, power);
		}
	}
////////////////////////////////////////////////////////////////
/////////////////////////G R A B
////////////////////////////////////////////////////////////////
	void GTSGrab_Catch_Actor(AnimationEventData& data) {
		auto giant = &data.giant;
		for (auto otherActor: find_actors()) {
			if (otherActor != giant) {
				float giantscale = get_visual_scale(giant);
				float victimscale = get_visual_scale(otherActor);
				float sizedifference = giantscale/victimscale;
				NiPoint3 giantLocation = giant->GetPosition();
				NiPoint3 tinyLocation = otherActor->GetPosition();
				if ((tinyLocation-giantLocation).Length() < 60*get_visual_scale(giant) && sizedifference >= 6.2) {
					ReportCrime(giant, otherActor, 10, false);
					Grab::GrabActor(giant, otherActor);
					break;
				}
			}
		}
	}

////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////
/////////////////////////A T T A C K
////////////////////////////////////////////////////////////////
	void GTSGrab_Attack_MoveStart(AnimationEventData& data) {
		auto& sizemanager = SizeManager::GetSingleton();
		auto giant = &data.giant;
		if (giant->formID == 0x14) {
			sizemanager.SetActionBool(giant, true, 7.0);
		}
	}

	void GTSGrab_Attack_Damage(AnimationEventData& data) {
		auto& sizemanager = SizeManager::GetSingleton();
		auto giant = &data.giant;
		auto grabbedActor = Grab::GetHeldActor(giant);
		if (!grabbedActor) {
			AnimationManager::StartAnim("GTSBEH_AbortGrab", giant);
			return;
		}
		if (grabbedActor) {
			Rumble::Once("GrabAttack", &data.giant, 4.0, 0.15, "NPC L Hand [LHnd]");
			float sd = get_visual_scale(giant)/get_visual_scale(grabbedActor);
			float Health = GetAV(grabbedActor, ActorValue::kHealth);
			float power = std::clamp(sizemanager.GetSizeAttribute(giant, 0), 1.0f, 999999.0f);
			float additionaldamage = 1.0 + sizemanager.GetSizeVulnerability(grabbedActor);
			float damage = (0.025 * sd) * power * additionaldamage;
			DamageAV(grabbedActor, ActorValue::kHealth, damage);
			auto root = find_node(grabbedActor, "NPC Root [Root]");
			if (root) {
				SpawnParticle(giant, 25.0, "GTS/Damage/Explode.nif", root->world.rotate, root->world.translate, get_visual_scale(grabbedActor), 4, root);
			}
			SizeHitEffects::GetSingleton().BreakBones(giant, grabbedActor, damage * 0.5, 25);
			if (damage > Health * 1.5) {
				CrushManager::Crush(giant, grabbedActor);
				PrintDeathSource(giant, grabbedActor, "HandCrushed");
				Grab::Release(giant);
			}
		}
	}

	void GTSGrab_Attack_MoveStop(AnimationEventData& data) {
		auto& sizemanager = SizeManager::GetSingleton();
		auto giant = &data.giant;
		if (giant->formID == 0x14) {
			sizemanager.SetActionBool(giant, false, 7.0);
		}
	}

////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////
/////////////////////////V O R E
////////////////////////////////////////////////////////////////
	void GTSGrab_Eat_Start(AnimationEventData& data) {
		auto otherActor = Grab::GetHeldActor(&data.giant);	
		ManageCamera(&data.giant, true, 2.0);
		if (otherActor) {
			voreData.AddTiny(otherActor);
		}
	}

	void GTSGrab_Eat_OpenMouth(AnimationEventData& data) {
		AdjustFacialExpression(&data.giant, 0, 1.0, "phenome"); // Start opening mouth
		AdjustFacialExpression(&data.giant, 1, 0.5, "phenome"); // Open it wider
	}

	void GTSGrab_Eat_Eat(AnimationEventData& data) {
		auto otherActor = Grab::GetHeldActor(&data.giant);			
		if (otherActor) {
			if (!AllowDevourment()) {
				auto& VoreData = Vore::GetSingleton().GetVoreData(&data.giant);
				Runtime::PlaySoundAtNode("VoreSwallow", giant, 1.0, 1.0, "NPC Head [Head]"); // Play sound
			} 
		}
	}

	void GTSGrab_Eat_CloseMouth(AnimationEventData& data) {
		AdjustFacialExpression(&data.giant, 0, 0.0, "phenome"); // Close it
		AdjustFacialExpression(&data.giant, 1, 0.0, "phenome"); // And close it
	}

	void GTSGrab_Eat_Swallow(AnimationEventData& data) {
		auto otherActor = Grab::GetHeldActor(&data.giant);			
		if (otherActor) {
			if (!AllowDevourment()) {
				auto& VoreData = Vore::GetSingleton().GetVoreData(&data.giant);
				VoreData.KillAll();
			} else {
				CallDevourment(&data.giant, otherActor);
			}
		}
		ManageCamera(&data.giant, false, 2.0);
	}
	

////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////
/////////////////////////T H R O W
////////////////////////////////////////////////////////////////
	void GTSGrab_Throw_ThrowActor(AnimationEventData& data) {

	}

////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////
/////////////////////////R E L E A S E 
////////////////////////////////////////////////////////////////
	void GTSGrab_Release_FreeActor(AnimationEventData& data) {
		auto giant = &data.giant
		auto grabbedActor = Grab::GetHeldActor(giant);
		if (grabbedActor) {
			Grab::Release(giant);
			PushActorAway(giant, grabbedActor, 1.0);
		}
	}
	

	void GrabOtherEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		AnimationManager::StartAnim("GrabSomeone", player);
	}

	void GrabAttackEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		auto grabbedActor = Grab::GetHeldActor(player);
		if (!grabbedActor) { 
			return;
		}
		AnimationManager::StartAnim("GrabEatSomeone", player);
	}

	void GrabVoreEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		auto grabbedActor = Grab::GetHeldActor(player);
		if (!grabbedActor) { 
			return;
		}
		float sizedifference = get_visual_scale(player)/get_visual_scale(grabbedActor);
		if (sizedifference < 6.0) {
			return;
		}
		AnimationManager::StartAnim("GrabDamageAttack", player);
	}

	void GrabThrowEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		auto grabbedActor = Grab::GetHeldActor(player);
		if (!grabbedActor) { 
			return;
		}
		AnimationManager::StartAnim("GrabThrowSomeone", player);
	}

	void GrabReleaseEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		auto grabbedActor = Grab::GetHeldActor(player);
		if (!grabbedActor) { 
			return;
		}
		AnimationManager::StartAnim("GrabReleasePunies", player);
	}
}

namespace Gts {
	Grab& Grab::GetSingleton() noexcept {
		static Grab instance;
		return instance;
	}

	std::string Grab::DebugName() {
		return "Grab";
	}

	void Grab::Update() {
		for (auto &[giant, data]: this->data) {
			if (!giant) {
				continue;
			}
			auto tiny = data.tiny;
			if (!tiny) {
				continue;
			}

			auto bone = find_node(giant, "NPC L Finger02 [LF02]");
			if (!bone) {
				return;
			}

			float giantScale = get_visual_scale(giant);

			NiPoint3 giantLocation = giant->GetPosition();
			NiPoint3 tinyLocation = tiny->GetPosition();

			tiny->SetPosition(bone->world.translate);
			Actor* tiny_is_actor = skyrim_cast<Actor*>(tiny);
			if (tiny_is_actor) {
				auto charcont = tiny_is_actor->GetCharController();
				if (charcont) {
					charcont->SetLinearVelocityImpl((0.0, 0.0, 0.0, 0.0)); // Needed so Actors won't fall down.
				}
			} else {
				// TODO: Work out method for generic objects
			}

			// TODO: Add escape

			////////////////////// Sermit's Note: please add it to Grab only, not Vore. And even then im not sure if we really need it.

			// if Escaped(giant, tiny, data.strength) {
			//   this->data.erase(giant);
			// }
		}
	}


	void Grab::GrabActor(Actor* giant, TESObjectREFR* tiny, float strength) {
		Grab::GetSingleton().data.try_emplace(giant, tiny, strength);
	}
	void Grab::GrabActor(Actor* giant, TESObjectREFR* tiny) {
		// Default strength 1.0: normal grab for actor of their size
		//
		Grab::GrabActor(giant, tiny, 1.0);
	}

	void Grab::Reset() {
		this->data.clear();
	}

	void Grab::ResetActor(Actor* actor) {
		this->data.erase(actor);
	}

	void Grab::Release(Actor* giant) {
		Grab::GetSingleton().data.erase(giant);
	}

	TESObjectREFR* Grab::GetHeldObj(Actor* giant) {
		try {
			auto& me = Grab::GetSingleton();
			return me.data.at(giant).tiny;
		} catch (std::out_of_range e) {
			return nullptr;
		}

	}
	Actor* Grab::GetHeldActor(Actor* giant) {
		auto obj = Grab::GetHeldObj(giant);
		Actor* actor = skyrim_cast<Actor*>(obj);
		if (actor) {
			return actor;
		} else {
			return nullptr;
		}
	}

	void Grab::RegisterEvents()  {
		InputManager::RegisterInputEvent("GrabOther", GrabOtherEvent);
		InputManager::RegisterInputEvent("GrabAttack", GrabAttackEvent);
		InputManager::RegisterInputEvent("GrabVore", GrabVoreEvent);
		InputManager::RegisterInputEvent("GrabThrow", GrabThrowEvent);
		InputManager::RegisterInputEvent("GrabRelease", GrabReleaseEvent);

		AnimationManager::RegisterEvent("GTSGrab_Catch_Actor", "Grabbing", GTSGrab_Catch_Actor);
		AnimationManager::RegisterEvent("GTSGrab_Attack_MoveStart", "Grabbing", GTSGrab_Attack_MoveStart);
		AnimationManager::RegisterEvent("GTSGrab_Attack_Damage", "Grabbing", GTSGrab_Attack_Damage);
		AnimationManager::RegisterEvent("GTSGrab_Attack_MoveStop", "Grabbing", GTSGrab_Attack_MoveStop);
		AnimationManager::RegisterEvent("GTSGrab_Eat_Start", "Grabbing", GTSGrab_Eat_Start);
		AnimationManager::RegisterEvent("GTSGrab_Eat_OpenMouth", "Grabbing", GTSGrab_Eat_OpenMouth);
		AnimationManager::RegisterEvent("GTSGrab_Eat_Eat", "Grabbing", GTSGrab_Eat_Eat);
		AnimationManager::RegisterEvent("GTSGrab_Eat_CloseMouth", "Grabbing", GTSGrab_Eat_CloseMouth);
		AnimationManager::RegisterEvent("GTSGrab_Eat_Swallow", "Grabbing", GTSGrab_Eat_Swallow);
		AnimationManager::RegisterEvent("GTSGrab_Throw_ThrowActor", "Grabbing", GTSGrab_Throw_ThrowActor);
		AnimationManager::RegisterEvent("GTSGrab_Release_FreeActor", "Grabbing", GTSGrab_Release_FreeActor);
		AnimationManager::RegisterEvent("GTSGrab_Catch_Actor", "Grabbing", GTSGrab_Catch_Actor);
	}

	void Grab::RegisterTriggers() {
		AnimationManager::RegisterTrigger("GrabSomeone", "Grabbing", "GTSBEH_GrabStart");
		AnimationManager::RegisterTrigger("GrabEatSomeone", "Grabbing", "GTSBEH_GrabVore");
		AnimationManager::RegisterTrigger("GrabDamageAttack", "Grabbing", "GTSBEH_GrabAttack");
		AnimationManager::RegisterTrigger("GrabThrowSomeone", "Grabbing", "GTSBEH_GrabThrow");
		AnimationManager::RegisterTrigger("GrabReleasePunies", "Grabbing", "GTSBEH_GrabRelease");
		AnimationManager::RegisterTrigger("GrabExit", "Grabbing", "GTSBEH_GrabExit");
		AnimationManager::RegisterTrigger("GrabAbort", "Grabbing", "GTSBEH_AbortGrab");
	}

	GrabData::GrabData(TESObjectREFR* tiny, float strength) : tiny(tiny), strength(strength) {
	}
}


//Beh's:
/*
	GTSBEH_GrabStart
	GTSBEH_GrabVore
	GTSBEH_GrabAttack
	GTSBEH_GrabThrow
	GTSBEH_GrabRelease

	GTSBeh_GrabExit
	GTSBEH_AbortGrab (Similar to GTSBEH_Exit but Grab only)
*/