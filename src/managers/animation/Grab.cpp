#include "managers/animation/AnimationManager.hpp"
#include "managers/animation/AttachPoint.hpp"
#include "managers/GrabAnimationController.hpp"
#include "managers/emotions/EmotionManager.hpp"
#include "managers/ShrinkToNothingManager.hpp"
#include "managers/damage/SizeHitEffects.hpp"
#include "managers/damage/LaunchActor.hpp"
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
#include "managers/vore.hpp"
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

	const std::string_view RNode = "NPC R Foot [Rft ]";
	const std::string_view LNode = "NPC L Foot [Lft ]";

	bool Escaped(Actor* giant, Actor* tiny, float strength) {
		float tiny_chance = ((rand() % 100000) / 100000.0f) * get_visual_scale(tiny);
		float giant_chance = ((rand() % 100000) / 100000.0f) * strength * get_visual_scale(giant);
		return (tiny_chance > giant_chance);
	}

	void SpawnHurtParticles(Actor* giant, Actor* grabbedActor, float mult, float dustmult) {
		auto hand = find_node(giant, "NPC L Hand [LHnd]");
		if (hand) {
			if (IsLiving(grabbedActor)) {
				if (!LessGore()) {
					SpawnParticle(giant, 25.0, "GTS/Damage/Explode.nif", hand->world.rotate, hand->world.translate, get_visual_scale(grabbedActor) * 3* mult, 4, hand);
					SpawnParticle(giant, 25.0, "GTS/Damage/Crush.nif", hand->world.rotate, hand->world.translate, get_visual_scale(grabbedActor) * 3 *  mult, 4, hand);
				} else if (LessGore()) {
					Runtime::PlaySound("BloodGushSound", grabbedActor, 1.0, 0.5);
				}
			} else {
				SpawnDustParticle(giant, grabbedActor, "NPC L Hand [LHnd]", dustmult);
			}
		}
	}

	void GrabStaminaDrain(Actor* giant, Actor* tiny, float sizedifference) {
		float WasteMult = 1.0;
		if (Runtime::HasPerkTeam(giant, "DestructionBasics")) {
			WasteMult *= 0.65;
		}
		float WasteStamina = (1.00 * WasteMult)/sizedifference;
		DamageAV(giant, ActorValue::kStamina, WasteStamina);
	}

	void DrainStamina(Actor* giant, std::string_view TaskName, bool decide, float power) {
		float WasteMult = 1.0;
		if (Runtime::HasPerkTeam(giant, "DestructionBasics")) {
			WasteMult *= 0.65;
		}
		std::string name = std::format("StaminaDrain_{}_{}", TaskName, giant->formID);
		if (decide) {
			TaskManager::Run(name, [=](auto& progressData) {
				ActorHandle casterhandle = giant->CreateRefHandle();
				if (!casterhandle) {
					return false;
				}
				float multiplier = AnimationManager::GetAnimSpeed(giant);
				float WasteStamina = 0.50 * power * multiplier;
				DamageAV(giant, ActorValue::kStamina, WasteStamina * WasteMult);
				return true;
			});
		} else {
			TaskManager::Cancel(name);
		}
	}

	void ToggleEmotionEdit(Actor* giant, bool allow) {
		auto& Emotions = EmotionManager::GetSingleton().GetGiant(giant);
		Emotions.AllowEmotionEdit = allow;
	}

	void DoLaunch(Actor* giant, float radius, float damage, std::string_view node) {
		float bonus = 1.0;
		if (HasSMT(giant)) {
			bonus = 2.0; // Needed to boost only Launch
		}
		LaunchActor::GetSingleton().ApplyLaunch(giant, radius * bonus, damage, node);
	}

	float GetPerkBonus(Actor* Giant) {
		if (Runtime::HasPerkTeam(Giant, "DestructionBasics")) {
			return 1.25;
		} else {
			return 1.0;
		}
	}

	void ManageCamera(Actor* giant, bool enable, float type) {
		auto& sizemanager = SizeManager::GetSingleton();
		if (giant->formID == 0x14) {
			if (AllowFeetTracking()) {
				sizemanager.SetActionBool(giant, enable, type);
			}
		}
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
	void GTSGrab_Catch_Start(AnimationEventData& data) {
		ManageCamera(&data.giant, true, 7.0);
		auto grabbedActor = Grab::GetHeldActor(&data.giant);
		if (grabbedActor) {
			SetBeingHeld(grabbedActor, true);
		}
	}

	void GTSGrab_Catch_Actor(AnimationEventData& data) {
		auto giant = &data.giant;
		giant->SetGraphVariableInt("GTS_GrabbedTiny", 1);
		auto grabbedActor = Grab::GetHeldActor(&data.giant);
		if (grabbedActor) {
			Grab::AttachActorTask(giant, grabbedActor);
		}
	}

	void GTSGrab_Catch_End(AnimationEventData& data) {
		ManageCamera(&data.giant, false, 7.0);
	}


////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////
/////////////////////////A T T A C K
////////////////////////////////////////////////////////////////
	void GTSGrab_Attack_MoveStart(AnimationEventData& data) {
		auto giant = &data.giant;
		DrainStamina(giant, "GrabAttack", true, 1.0);
		ManageCamera(giant, true, 7.0);
	}

	void GTSGrab_Attack_Damage(AnimationEventData& data) {
		auto& sizemanager = SizeManager::GetSingleton();
		float bonus = 1.0;
		auto giant = &data.giant;
		auto grabbedActor = Grab::GetHeldActor(giant);

		static Timer laughtimer = Timer(6.0);
		if (grabbedActor) {
			StartCombat(giant, grabbedActor, true); // force combat
			float sd = get_visual_scale(giant)/get_visual_scale(grabbedActor);
			float Health = GetAV(grabbedActor, ActorValue::kHealth);
			float power = std::clamp(sizemanager.GetSizeAttribute(giant, 0), 1.0f, 999999.0f);
			float additionaldamage = 1.0 + sizemanager.GetSizeVulnerability(grabbedActor);
			float damage = (1.400 * sd) * power * additionaldamage * additionaldamage;
			if (HasSMT(giant)) {
				damage *= 1.75;
				bonus = 3.0;
			}
			DamageAV(grabbedActor, ActorValue::kHealth, damage);
			Rumble::Once("GrabAttack", giant, 6.0 * bonus, 0.05, "NPC L Hand [LHnd]");
			SizeHitEffects::GetSingleton().BreakBones(giant, grabbedActor, 0, 1);
			AdjustGtsSkill(damage/2500, giant);
			if (damage < Health) {
				if (!LessGore()) {
					Runtime::PlaySoundAtNode("CrunchImpactSound", giant, 1.0, 0.0, "NPC L Hand [LHnd]");
					SpawnHurtParticles(giant, grabbedActor, 1.0, 1.0);
				} else {
					Runtime::PlaySoundAtNode("SoftHandAttack", giant, 1.0, 0.0, "NPC L Hand [LHnd]");
				}
			}
			if (damage > Health) {
				CrushManager::Crush(giant, grabbedActor);
				SetBeingHeld(grabbedActor, false);
				Rumble::Once("GrabAttackKill", giant, 16.0 * bonus, 0.15, "NPC L Hand [LHnd]");
				if (!LessGore()) {
					Runtime::PlaySoundAtNode("CrunchImpactSound", giant, 1.0, 0.0, "NPC L Hand [LHnd]");
					Runtime::PlaySoundAtNode("CrunchImpactSound", giant, 1.0, 0.0, "NPC L Hand [LHnd]");
					Runtime::PlaySoundAtNode("CrunchImpactSound", giant, 1.0, 0.0, "NPC L Hand [LHnd]");
				} else {
					Runtime::PlaySoundAtNode("SoftHandAttack", giant, 1.0, 0.0, "NPC L Hand [LHnd]");
				}
				ReportCrime(giant, grabbedActor, 1000.0, true); // Report Crime since we killed someone
				SpawnHurtParticles(giant, grabbedActor, 3.0, 1.6);
				SpawnHurtParticles(giant, grabbedActor, 3.0, 1.6);
				giant->SetGraphVariableInt("GTS_Grab_State", 0);
				PrintDeathSource(giant, grabbedActor, "HandCrushed");
				AnimationManager::StartAnim("TinyDied", giant);
				Grab::DetachActorTask(giant);
				Grab::Release(giant);
			}
		}
	}

	void GTSGrab_Attack_MoveStop(AnimationEventData& data) {
		auto giant = &data.giant;
		auto& sizemanager = SizeManager::GetSingleton();
		auto grabbedActor = Grab::GetHeldActor(giant);
		ManageCamera(giant, false, 7.0);
		DrainStamina(giant, "GrabAttack", false, 1.0);
		if (!grabbedActor) {
			log::info("GrabbedActor is null");
			giant->SetGraphVariableInt("GTS_GrabbedTiny", 0);
			AnimationManager::StartAnim("GrabAbort", giant);
			AnimationManager::StartAnim("TinyDied", giant);
			Grab::DetachActorTask(giant);
			Grab::Release(giant);
		}
	}

////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////
/////////////////////////V O R E
////////////////////////////////////////////////////////////////
	void GTSGrab_Eat_Start(AnimationEventData& data) {
		ToggleEmotionEdit(&data.giant, true);
		auto otherActor = Grab::GetHeldActor(&data.giant);
		auto& VoreData = Vore::GetSingleton().GetVoreData(&data.giant);
		ManageCamera(&data.giant, true, 7.0);
		if (otherActor) {
			VoreData.AddTiny(otherActor);
		}
	}

	void GTSGrab_Eat_OpenMouth(AnimationEventData& data) {
		auto giant = &data.giant;
		auto otherActor = Grab::GetHeldActor(giant);
		auto& VoreData = Vore::GetSingleton().GetVoreData(giant);
		if (otherActor) {
			SetBeingEaten(otherActor, true);
		}
		AdjustFacialExpression(giant, 0, 1.0, "phenome"); // Start opening mouth
		AdjustFacialExpression(giant, 1, 0.5, "phenome"); // Open it wider

		AdjustFacialExpression(giant, 0, 0.80, "modifier"); // blink L
		AdjustFacialExpression(giant, 1, 0.80, "modifier"); // blink R

		AdjustFacialExpression(&data.giant, 3, 0.8, "phenome"); // Smile a bit (Mouth)
	}

	void GTSGrab_Eat_Eat(AnimationEventData& data) {
		auto otherActor = Grab::GetHeldActor(&data.giant);
		auto& VoreData = Vore::GetSingleton().GetVoreData(&data.giant);
		if (otherActor) {
			for (auto& tiny: VoreData.GetVories()) {
				if (!AllowDevourment()) {
					VoreData.Swallow();
				} else {
					CallDevourment(&data.giant, otherActor);
				}
			}
		}
	}

	void GTSGrab_Eat_CloseMouth(AnimationEventData& data) {
		auto giant = &data.giant;
		AdjustFacialExpression(giant, 0, 0.0, "phenome"); // Close mouth
		AdjustFacialExpression(giant, 1, 0.0, "phenome"); // Close it

		AdjustFacialExpression(giant, 0, 0.0, "modifier"); // blink L
		AdjustFacialExpression(giant, 1, 0.0, "modifier"); // blink R

		AdjustFacialExpression(&data.giant, 3, 0.0, "phenome"); // Smile a bit (Mouth)
	}

	void GTSGrab_Eat_Swallow(AnimationEventData& data) {
		auto giant = &data.giant;
		auto otherActor = Grab::GetHeldActor(&data.giant);
		if (otherActor) {
			SetBeingEaten(otherActor, false);
			auto& VoreData = Vore::GetSingleton().GetVoreData(&data.giant);
			for (auto& tiny: VoreData.GetVories()) {
				VoreData.KillAll();
			}
			giant->SetGraphVariableInt("GTS_GrabbedTiny", 0);
			Runtime::PlaySoundAtNode("VoreSwallow", &data.giant, 1.0, 1.0, "NPC Head [Head]"); // Play sound
			giant->SetGraphVariableInt("GTS_Grab_State", 0);
			AnimationManager::StartAnim("TinyDied", giant);
			ManageCamera(&data.giant, false, 7.0);
			SetBeingHeld(otherActor, false);
			Grab::DetachActorTask(giant);
			Grab::Release(giant);
		}
		ToggleEmotionEdit(giant, false);
	}


////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////
/////////////////////////T H R O W
////////////////////////////////////////////////////////////////
	void GTSGrab_Throw_MoveStart(AnimationEventData& data) {
		auto giant = &data.giant;
		DrainStamina(giant, "GrabThrow", true, 1.8);
		ManageCamera(giant, true, 7.0);
	}

	void GTSGrab_Throw_FS_R(AnimationEventData& data) {
		float shake = 1.0;
		float launch = 1.0;
		float dust = 1.0;
		float perk = GetPerkBonus(&data.giant);
		if (Runtime::HasMagicEffect(&data.giant, "SmallMassiveThreat")) {
			shake = 4.0;
			launch = 1.5;
			dust = 1.25;
		}
		Rumble::Once("StompR", &data.giant, 1.50 * shake, 0.0, RNode);
		DoDamageEffect(&data.giant, 1.1 * launch * data.animSpeed * perk, 1.0 * launch * data.animSpeed, 10, 0.25);
		DoSizeEffect(&data.giant, 0.9 * data.animSpeed, FootEvent::Right, RNode, dust);
		DoLaunch(&data.giant, 0.8 * launch, 1.75, RNode);
	}

	void GTSGrab_Throw_FS_L(AnimationEventData& data) {
		float shake = 1.0;
		float launch = 1.0;
		float dust = 1.0;
		float perk = GetPerkBonus(&data.giant);
		if (Runtime::HasMagicEffect(&data.giant, "SmallMassiveThreat")) {
			shake = 4.0;
			launch = 1.5;
			dust = 1.25;
		}
		Rumble::Once("StompL", &data.giant, 1.50 * shake, 0.0, LNode);
		DoDamageEffect(&data.giant, 1.1 * launch * data.animSpeed * perk, 1.0 * launch * data.animSpeed, 10, 0.25);
		DoSizeEffect(&data.giant, 0.9 * data.animSpeed, FootEvent::Left, LNode, dust);
		DoLaunch(&data.giant, 0.8 * launch * perk, 1.75, LNode);
	}

	void GTSGrab_Throw_Throw_Pre(AnimationEventData& data) {// Throw frame 0
	}

	void GTSGrab_Throw_ThrowActor(AnimationEventData& data) { // Throw frame 1
		auto giant = &data.giant;
		auto otherActor = Grab::GetHeldActor(&data.giant);
		if (otherActor) {
			SetBeingHeld(otherActor, false);
		}
        giant->SetGraphVariableInt("GTS_GrabbedTiny", 0);
		giant->SetGraphVariableInt("GTS_Grab_State", 0);
		ManageCamera(giant, false, 7.0);
		AnimationManager::StartAnim("TinyDied", giant);
		Grab::DetachActorTask(giant);
		Grab::Release(giant);

	}

	void GTSGrab_Throw_Throw_Post(AnimationEventData& data) { // Throw frame 2
	}

	void GTSGrab_Throw_MoveStop(AnimationEventData& data) {
		auto giant = &data.giant;
		DrainStamina(giant, "GrabThrow", false, 1.8);
	}


////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////
/////////////////////////R E L E A S E
////////////////////////////////////////////////////////////////
	void GTSGrab_Release_FreeActor(AnimationEventData& data) {
		auto giant = &data.giant;
		SetBetweenBreasts(giant, false);
		giant->SetGraphVariableInt("GTS_GrabbedTiny", 0);
		giant->SetGraphVariableInt("GTS_GrabbedTiny", 0);
		auto grabbedActor = Grab::GetHeldActor(giant);
		ManageCamera(&data.giant, false, 7.0);
		AnimationManager::StartAnim("TinyDied", giant);
		Grab::DetachActorTask(giant);
		Grab::Release(giant);
		if (grabbedActor) {
			PushActorAway(giant, grabbedActor, 0.1);
		}
	}

	void GTSBEH_GrabExit(AnimationEventData& data) {
		auto giant = &data.giant;
		SetBetweenBreasts(giant, false);
		giant->SetGraphVariableInt("GTS_GrabbedTiny", 0);
		giant->SetGraphVariableInt("GTS_GrabbedTiny", 0);
		AnimationManager::StartAnim("TinyDied", giant);
		DrainStamina(giant, "GrabAttack", false, 1.0);
		ManageCamera(&data.giant, false, 7.0);
		Grab::DetachActorTask(giant);
		Grab::Release(giant);
	}

	void GTSBEH_AbortGrab(AnimationEventData& data) {
		auto giant = &data.giant;
		SetBetweenBreasts(giant, false);
		giant->SetGraphVariableInt("GTS_GrabbedTiny", 0);
		giant->SetGraphVariableInt("GTS_Storing_Tiny", 0);
		AnimationManager::StartAnim("TinyDied", giant);
		DrainStamina(giant, "GrabAttack", false, 1.0);
		ManageCamera(&data.giant, false, 7.0);
		Grab::DetachActorTask(giant);
		Grab::Release(giant);
	}

	/////////////////////////////////////////////////////////////////
	////////////////////////////B R E A S T S
	/////////////////////////////////////////////////////////////////

	void GTSGrab_Breast_MoveStart(AnimationEventData& data) {
		ManageCamera(&data.giant, true, 7.0);
	}

	void GTSGrab_Breast_PutActor(AnimationEventData& data) { // Places actor between breasts
		auto giant = &data.giant;
		SetBetweenBreasts(giant, true);
		Runtime::PlaySoundAtNode("BreastImpact", giant, 1.0, 0.0, "NPC L Hand [LHnd]");
		giant->SetGraphVariableInt("GTS_Storing_Tiny", 1);
		giant->SetGraphVariableInt("GTS_GrabbedTiny", 0);
	}

	void GTSGrab_Breast_TakeActor(AnimationEventData& data) { // Removes Actor
		auto giant = &data.giant;
		SetBetweenBreasts(giant, false);
		giant->SetGraphVariableInt("GTS_Storing_Tiny", 0);
		giant->SetGraphVariableInt("GTS_GrabbedTiny", 1);
	}

	void GTSGrab_Breast_MoveEnd(AnimationEventData& data) {
		ManageCamera(&data.giant, false, 7.0);
	}

	///////////////////////////////////////////////////////////////////
	/////////////////////////////E V E N T S
	///////////////////////////////////////////////////////////////////

	void GrabOtherEvent(const InputEventData& data) { // Grab other actor
		auto player = PlayerCharacter::GetSingleton();
		if (IsEquipBusy(player)) {
			return; // Disallow Grabbing if Behavior is busy doing other stuff.
		}
		auto& Grabbing = GrabAnimationController::GetSingleton();
		std::size_t numberOfPrey = 1;
		if (Runtime::HasPerkTeam(player, "MassVorePerk")) {
			numberOfPrey = 1 + (get_visual_scale(player)/3);
		}
		std::vector<Actor*> preys = Grabbing.GetGrabTargetsInFront(player, numberOfPrey);
		for (auto prey: preys) {
			Grabbing.StartGrab(player, prey);
		}
	}

	void GrabAttackEvent(const InputEventData& data) { // Attack everyone in your hand
		auto player = PlayerCharacter::GetSingleton();
		auto grabbedActor = Grab::GetHeldActor(player);
		if (!grabbedActor) {
			return;
		}
		float WasteStamina = 20.0;
		if (Runtime::HasPerk(player, "DestructionBasics")) {
			WasteStamina *= 0.65;
		} if (GetAV(player, ActorValue::kStamina) > WasteStamina) {
			AnimationManager::StartAnim("GrabDamageAttack", player);
		} else {
			TiredSound(player, "You're too tired to perform hand attack");
		}
	}

	void GrabVoreEvent(const InputEventData& data) { // Eat everyone in hand
		auto player = PlayerCharacter::GetSingleton();
		auto grabbedActor = Grab::GetHeldActor(player);
		if (!grabbedActor) {
			return;
		}
		AnimationManager::StartAnim("GrabEatSomeone", player);
	}

	void GrabThrowEvent(const InputEventData& data) { // Throw everyone away
		auto player = PlayerCharacter::GetSingleton();
		auto grabbedActor = Grab::GetHeldActor(player);
		if (!grabbedActor) {
			return;
		}
		float WasteStamina = 40.0;
		if (Runtime::HasPerk(player, "DestructionBasics")) {
			WasteStamina *= 0.65;
		} if (GetAV(player, ActorValue::kStamina) > WasteStamina) {
			AnimationManager::StartAnim("GrabThrowSomeone", player);
		} else {
			TiredSound(player, "You're too tired to throw that actor");
		}
	}

	void GrabReleaseEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		auto grabbedActor = Grab::GetHeldActor(player);
		if (!grabbedActor) {
			return;
		}
		if (IsGtsBusy(player)) {
			return;
		}
		AnimationManager::StartAnim("GrabReleasePunies", player);
	}

	void BreastsPutEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		auto grabbedActor = Grab::GetHeldActor(player);
		if (!grabbedActor) {
			return;
		}
		AnimationManager::StartAnim("Breasts_Put", player);
	}
    void BreastsRemoveEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		auto grabbedActor = Grab::GetHeldActor(player);
		if (!grabbedActor) {
			return;
		}
		AnimationManager::StartAnim("Breasts_Pull", player);
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

	void Grab::DetachActorTask(Actor* giant) {
		std::string name = std::format("GrabAttach_{}", giant->formID);
		giant->SetGraphVariableInt("GTS_GrabbedTiny", 0); // Tell behaviors 'we have nothing in our hands'. A must.
		giant->SetGraphVariableInt("GTS_Grab_State", 0);
		TaskManager::Cancel(name);
	}

	void Grab::AttachActorTask(Actor* giant, Actor* tiny) {
		if (!giant) {
			return;
		}
		if (!tiny) {
			return;
		}
		std::string name = std::format("GrabAttach_{}", giant->formID);
		ActorHandle gianthandle = giant->CreateRefHandle();
		ActorHandle tinyhandle = tiny->CreateRefHandle();
		TaskManager::Run(name, [=](auto& progressData) {
			auto giantref = gianthandle.get().get();
			auto tinyref = tinyhandle.get().get();

			// Exit on death
			float sizedifference = get_target_scale(giantref)/get_target_scale(tinyref);

			if (tinyref->IsDead() || sizedifference < 6.0 || GetAV(giantref, ActorValue::kStamina) < 2.0) {
				log::info("{} is small/dead", tinyref->GetDisplayFullName());
				Grab::Release(giantref);
				giantref->SetGraphVariableInt("GTS_GrabbedTiny", 0); // Tell behaviors 'we have nothing in our hands'. A must.
				giantref->SetGraphVariableInt("GTS_Grab_State", 0);
				DrainStamina(giantref, "GrabAttack", false, 1.0);
				AnimationManager::StartAnim("GrabAbort", giantref); // Abort Grab animation
				AnimationManager::StartAnim("TinyDied", giant);
				ManageCamera(giantref, false, 7.0); // Disable any camera edits
				return false;
			}

			if (IsBeingEaten(tinyref)) {
				if (!AttachToObjectA(gianthandle, tinyhandle)) {
				// Unable to attach
				return false;
			}
			} else if (IsBetweenBreasts(giantref)) {
			if (!AttachToCleavage(gianthandle, tinyhandle)) {
				// Unable to attach
				return false;
				}
			} else if (AttachToHand(gianthandle, tinyhandle)) {
				GrabStaminaDrain(giantref, tinyref, sizedifference);
				return true;
			} else {
			if (!AttachToHand(gianthandle, tinyhandle)) {
				// Unable to attach
				return false;
			}
      	}	

      // All good try another frame
			return true;
		});
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

	void Grab::RegisterEvents() {
		InputManager::RegisterInputEvent("GrabOther", GrabOtherEvent);
		InputManager::RegisterInputEvent("GrabAttack", GrabAttackEvent);
		InputManager::RegisterInputEvent("GrabVore", GrabVoreEvent);
		InputManager::RegisterInputEvent("GrabThrow", GrabThrowEvent);
		InputManager::RegisterInputEvent("GrabRelease", GrabReleaseEvent);
		InputManager::RegisterInputEvent("BreastsPut", BreastsPutEvent);
		InputManager::RegisterInputEvent("BreastsRemove", BreastsRemoveEvent);

		AnimationManager::RegisterEvent("GTSGrab_Catch_Start", "Grabbing", GTSGrab_Catch_Start);
		AnimationManager::RegisterEvent("GTSGrab_Catch_Actor", "Grabbing", GTSGrab_Catch_Actor);
		AnimationManager::RegisterEvent("GTSGrab_Catch_End", "Grabbing", GTSGrab_Catch_End);

		AnimationManager::RegisterEvent("GTSGrab_Attack_MoveStart", "Grabbing", GTSGrab_Attack_MoveStart);
		AnimationManager::RegisterEvent("GTSGrab_Attack_Damage", "Grabbing", GTSGrab_Attack_Damage);
		AnimationManager::RegisterEvent("GTSGrab_Attack_MoveStop", "Grabbing", GTSGrab_Attack_MoveStop);

		AnimationManager::RegisterEvent("GTSGrab_Eat_Start", "Grabbing", GTSGrab_Eat_Start);
		AnimationManager::RegisterEvent("GTSGrab_Eat_OpenMouth", "Grabbing", GTSGrab_Eat_OpenMouth);
		AnimationManager::RegisterEvent("GTSGrab_Eat_Eat", "Grabbing", GTSGrab_Eat_Eat);
		AnimationManager::RegisterEvent("GTSGrab_Eat_CloseMouth", "Grabbing", GTSGrab_Eat_CloseMouth);
		AnimationManager::RegisterEvent("GTSGrab_Eat_Swallow", "Grabbing", GTSGrab_Eat_Swallow);

		AnimationManager::RegisterEvent("GTSGrab_Throw_MoveStart", "Grabbing", GTSGrab_Throw_MoveStart);
		AnimationManager::RegisterEvent("GTSGrab_Throw_FS_R", "Grabbing", GTSGrab_Throw_FS_R);
		AnimationManager::RegisterEvent("GTSGrab_Throw_FS_L", "Grabbing", GTSGrab_Throw_FS_L);
		AnimationManager::RegisterEvent("GTSGrab_Throw_Throw_Pre", "Grabbing", GTSGrab_Throw_Throw_Pre);
		AnimationManager::RegisterEvent("GTSGrab_Throw_ThrowActor", "Grabbing", GTSGrab_Throw_ThrowActor);
		AnimationManager::RegisterEvent("GTSGrab_Throw_Throw_Post", "Grabbing", GTSGrab_Throw_Throw_Post);
		AnimationManager::RegisterEvent("GTSGrab_Throw_MoveStop", "Grabbing", GTSGrab_Throw_MoveStop);

		AnimationManager::RegisterEvent("GTSGrab_Breast_MoveStart", "Grabbing", GTSGrab_Breast_MoveStart);
		AnimationManager::RegisterEvent("GTSGrab_Breast_PutActor", "Grabbing", GTSGrab_Breast_PutActor);
		AnimationManager::RegisterEvent("GTSGrab_Breast_TakeActor", "Grabbing", GTSGrab_Breast_TakeActor);
		AnimationManager::RegisterEvent("GTSGrab_Breast_MoveEnd", "Grabbing", GTSGrab_Breast_MoveEnd);

		AnimationManager::RegisterEvent("GTSGrab_Release_FreeActor", "Grabbing", GTSGrab_Release_FreeActor);

		AnimationManager::RegisterEvent("GTSBEH_GrabExit", "Grabbing", GTSBEH_GrabExit);
		AnimationManager::RegisterEvent("GTSBEH_AbortGrab", "Grabbing", GTSBEH_AbortGrab);
	}

	void Grab::RegisterTriggers() {
		AnimationManager::RegisterTrigger("GrabSomeone", "Grabbing", "GTSBEH_GrabStart");
		AnimationManager::RegisterTrigger("GrabEatSomeone", "Grabbing", "GTSBEH_GrabVore");
		AnimationManager::RegisterTrigger("GrabDamageAttack", "Grabbing", "GTSBEH_GrabAttack");
		AnimationManager::RegisterTrigger("GrabThrowSomeone", "Grabbing", "GTSBEH_GrabThrow");
		AnimationManager::RegisterTrigger("GrabReleasePunies", "Grabbing", "GTSBEH_GrabRelease");
		AnimationManager::RegisterTrigger("GrabExit", "Grabbing", "GTSBEH_GrabExit");
		AnimationManager::RegisterTrigger("GrabAbort", "Grabbing", "GTSBEH_AbortGrab");
		AnimationManager::RegisterTrigger("TinyDied", "Grabbing", "GTSBEH_TinyDied");
		AnimationManager::RegisterTrigger("Breasts_Put", "Grabbing", "GTSBEH_BreastsAdd");
		AnimationManager::RegisterTrigger("Breasts_Pull", "Grabbing", "GTSBEH_BreastsRemove");
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
