#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/animation/Controllers/HugController.hpp"
#include "managers/emotions/EmotionManager.hpp"
#include "managers/ShrinkToNothingManager.hpp"
#include "managers/damage/SizeHitEffects.hpp"
#include "managers/damage/LaunchActor.hpp"
#include "managers/animation/HugShrink.hpp"
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
#include "colliders/charcontroller.hpp"

#include <random>

using namespace RE;
using namespace REL;
using namespace Gts;
using namespace std;


namespace {
	void GreetingStuff(Actor* actor) {
		auto ai = actor->GetActorRuntimeData().currentProcess->high;
		if (ai) {
			float Greeting = ai->greetingTimer;
			ai->greetingTimer = 40;
			log::info("Greeting timer of {} is {}", actor->GetDisplayFullName(), Greeting);
		}
	}

	float GetStealRate(Actor* actor) {
		float steal = 0.20;
		if (Runtime::HasPerkTeam(actor, "HugCrush")) {
			steal *= 1.35;
		}
		return steal;
	}
	float GetShrinkThreshold(Actor* actor) {
		float threshold = 2.5;
		float bonus = 1.0;
		if (Runtime::HasPerk(actor, "HugCrush")) {
			bonus += 0.25;
		} if (Runtime::HasPerk(actor, "HugCrush_Greed")) {
			bonus += 0.35;
		}
		return threshold * bonus;
	}

	void GTS_Hug_Grab(AnimationEventData& data) {
		auto giant = &data.giant;
		auto huggedActor = HugShrink::GetHuggiesActor(giant);
		if (!huggedActor) {
			return;
		}
		ToggleEmotionEdit(giant, true);
		SetBeingHeld(huggedActor, true);
		AllowDialogue(huggedActor, false);
		HugShrink::AttachActorTask(giant, huggedActor);

		float sizedifference = get_visual_scale(giant)/get_visual_scale(huggedActor);
		if (giant->formID == 0x14) {
			shake_camera(giant, 0.70 * sizedifference, 0.35);
		} else {
			Rumble::Once("HugGrab", giant, sizedifference * 12, 0.15);
		}

   		DisableCollisions(huggedActor, giant);
	}

	void GTS_Hug_Grow(AnimationEventData& data) {
		auto giant = &data.giant;
		auto huggedActor = HugShrink::GetHuggiesActor(giant);
		if (!huggedActor) {
			return;
		}
		HugShrink::ShrinkOtherTask(giant, huggedActor);
	}

	void GTS_Hug_Moan(AnimationEventData& data) {
		auto giant = &data.giant;
		AdjustFacialExpression(giant, 0, 1.0, "modifier"); // blink L
		AdjustFacialExpression(giant, 1, 1.0, "modifier"); // blink R
		AdjustFacialExpression(giant, 0, 0.75, "phenome");
		Runtime::PlaySoundAtNode("MoanSound", giant, 1.0, 1.0, "NPC Head [Head]");
	}

	void GTS_Hug_Moan_End(AnimationEventData& data) {
		auto giant = &data.giant;
		AdjustFacialExpression(giant, 0, 0.0, "modifier"); // blink L
		AdjustFacialExpression(giant, 1, 0.0, "modifier"); // blink R
		AdjustFacialExpression(giant, 0, 0.0, "phenome");
		Runtime::PlaySoundAtNode("MoanSound", giant, 1.0, 1.0, "NPC Head [Head]");
	}

	void GTS_Hug_FacialOn(AnimationEventData& data) { // Smug or something
		AdjustFacialExpression(&data.giant, 2, 1.0, "expression");
	}

	void GTS_Hug_FacialOff(AnimationEventData& data) { // Disable smug
		AdjustFacialExpression(&data.giant, 2, 0.0, "expression");
	}

	void GTS_Hug_PullBack(AnimationEventData& data) { // When we pull actor back to chest, used to play moans/laugh
		int Random = rand() % 5 + 1;
		if (Random >= 5) {
			Runtime::PlaySoundAtNode("LaughSound", &data.giant, 1.0, 1.0, "NPC Head [Head]");
		}
	}

	void GTSBEH_HugAbsorbAtk(AnimationEventData& data) {
		auto giant = &data.giant;
	}

	void GTS_Hug_ShrinkPulse(AnimationEventData& data) {
		auto giant = &data.giant;
		auto huggedActor = HugShrink::GetHuggiesActor(giant);
		if (!huggedActor) {
			return;
		}
		auto scale = get_visual_scale(huggedActor);
		float sizedifference = get_visual_scale(giant)/scale;
		set_target_scale(huggedActor, scale*0.60);
		Rumble::For("ShrinkPulse", giant, 6.0 * sizedifference, 0.10, "NPC COM [COM ]", 0.25);
	}

	void GTS_Hug_RunShrinkTask(AnimationEventData& data) {
		
	}

	void GTS_Hug_StopShrinkTask(AnimationEventData& data) {

	}

	void GTS_Hug_CrushTiny(AnimationEventData& data) {
		auto giant = &data.giant;
		auto huggedActor = HugShrink::GetHuggiesActor(giant);
		if (!huggedActor) {
			return;
		}
		ShrinkToNothingManager::Shrink(giant, huggedActor);
		PrintDeathSource(giant, huggedActor, "Shrinked");
		Rumble::For("HugCrush", giant, 36.0, 0.10, "NPC COM [COM ]", 0.15);
	}

  // Cancel all the things
  void AbortAnimation(Actor* giant, Actor* tiny) {
    AnimationManager::StartAnim("Huggies_Spare", giant);
	AdjustFacialExpression(giant, 0, 0.0, "phenome");
	AdjustFacialExpression(giant, 0, 0.0, "modifier");
	AdjustFacialExpression(giant, 1, 0.0, "modifier");
	HugShrink::Release(giant);
	if (tiny) {
		EnableCollisions(tiny);
		SetBeingHeld(tiny, false);
		AllowDialogue(tiny, true);
		PushActorAway(giant, tiny, 1.0);
	}
  }

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////// I N P U T
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	void HugAttemptEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		if (IsGtsBusy(player)) {
			return;
		}
		auto& Hugging = HugAnimationController::GetSingleton();
		std::size_t numberOfPrey = 1;
		if (Runtime::HasPerkTeam(player, "MassVorePerk")) {
			numberOfPrey = 1 + (get_visual_scale(player)/3);
		}
		std::vector<Actor*> preys = Hugging.GetHugTargetsInFront(player, numberOfPrey);
		for (auto prey: preys) {
			Hugging.StartHug(player, prey);
		}
	}

	void HugCrushEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		auto huggedActor = HugShrink::GetHuggiesActor(player);
		if (!huggedActor) {
			return;
		}
		AnimationManager::StartAnim("Huggies_HugCrush", player);
		AnimationManager::StartAnim("Huggies_HugCrush_Victim", huggedActor);
	}

	void HugShrinkEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		auto huggedActor = HugShrink::GetHuggiesActor(player);
		if (!huggedActor) {
			return;
		}
		if (get_target_scale(player)/get_target_scale(huggedActor) >= GetShrinkThreshold(player)) {
			AbortAnimation(player, huggedActor);
			return;
		}
		AnimationManager::StartAnim("Huggies_Shrink", player);
		AnimationManager::StartAnim("Huggies_Shrink_Victim", huggedActor);
	}
	void HugReleaseEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		auto huggedActor = HugShrink::GetHuggiesActor(player);
		AbortAnimation(player, huggedActor);
    	HugShrink::DetachActorTask(player);
	}
}

namespace Gts {
	HugShrink& HugShrink::GetSingleton() noexcept {
		static HugShrink instance;
		return instance;
	}

	std::string HugShrink::DebugName() {
		return "HugShrink";
	}

	void HugShrink::DetachActorTask(Actor* giant) {
		std::string name = std::format("Huggies_{}", giant->formID);
		std::string name_2 = std::format("Huggies_Shrink_{}", giant->formID);
		TaskManager::Cancel(name);
		TaskManager::Cancel(name_2);
	}

	void HugShrink::ShrinkOtherTask(Actor* giant, Actor* tiny) {
		if (!giant) {
			return;
		}
		if (!tiny) {
			return;
		}
		std::string name = std::format("Huggies_Shrink_{}", giant->formID);
		ActorHandle gianthandle = giant->CreateRefHandle();
		ActorHandle tinyhandle = tiny->CreateRefHandle();
		const float duration = 2.0;
		TaskManager::RunFor(name, duration, [=](auto& progressData) {
			if (!gianthandle) {
				return false;
			}
			if (!tinyhandle) {
				return false;
			}
			auto giantref = gianthandle.get().get();
			auto tinyref = tinyhandle.get().get();
			float sizedifference = get_target_scale(giantref)/get_target_scale(tinyref);
			float threshold = GetShrinkThreshold(giantref);
			float stamina = 0.35;
			float steal = GetStealRate(giantref);
			float shrink = 5.60;
			if (Runtime::HasPerkTeam(giantref, "HugCrush_Greed")) {
				shrink *= 1.25;
				stamina *= 0.65;
			}
			if (sizedifference >= threshold) {
				SetBeingHeld(tinyref, false);
				std::string_view message = std::format("You can't shrink {} any further", tinyref->GetDisplayFullName());
				Notify(message);
				AbortAnimation(giantref, tinyref);
				return false;
			}
			DamageAV(tinyref, ActorValue::kStamina, 0.60 * TimeScale()); // Drain Stamina
			DamageAV(giantref, ActorValue::kStamina, 0.25 * TimeScale()); // Damage GTS Stamina
			
			TransferSize(giantref, tinyref, false, shrink, steal, false); // Shrink foe, enlarge gts
			if (giantref->formID == 0x14) {
				shake_camera(giantref, 0.70 * sizedifference, 0.05);
			} else {
				Rumble::Once("HugSteal", giantref, get_visual_scale(giantref) * 8, 0.10);
			}
			return true;
		});
	}

	void HugShrink::AttachActorTask(Actor* giant, Actor* tiny) {
		if (!giant) {
			return;
		}
		if (!tiny) {
			return;
		}
		std::string name = std::format("Huggies_{}", giant->formID);
		ActorHandle gianthandle = giant->CreateRefHandle();
		ActorHandle tinyhandle = tiny->CreateRefHandle();
		TaskManager::Run(name, [=](auto& progressData) {
			if (!gianthandle) {
				return false;
			}
			if (!tinyhandle) {
				return false;
			}
			auto giantref = gianthandle.get().get();
			auto tinyref = tinyhandle.get().get();
			
			GreetingStuff(tinyref);

			// Exit on death
			float sizedifference = get_visual_scale(giantref)/get_visual_scale(tinyref);
			if (!FaceOpposite(giantref, tinyref)) {
				// If face towards fails then actor is invalid
				return false;
			}

			GrabStaminaDrain(giantref, tinyref, sizedifference * 2.6);
			AdjustGtsSkill(0.0001, giantref);

			DamageAV(tinyref, ActorValue::kStamina, 0.15 * TimeScale()); // Drain Tiny Stamina

			float stamina = GetAV(giantref, ActorValue::kStamina);
			if (giantref->IsDead() || tinyref->IsDead() || stamina <= 2.0 || !HugShrink::GetHuggiesActor(giantref)) {
				AbortAnimation(giantref, tinyref);
				return false;
			}
      		// Ensure they are NOT in ragdoll
      		ForceRagdoll(tinyref, false);
			if (!HugAttach(gianthandle, tinyhandle)) {
				return false;
			}

			// All good try another frame
			return true;
		});
	}


	void HugShrink::HugActor_Actor(Actor* giant, TESObjectREFR* tiny, float strength) {
		HugShrink::GetSingleton().data.try_emplace(giant, tiny, strength);
	}
	void HugShrink::HugActor(Actor* giant, TESObjectREFR* tiny) {
		// Default strength 1.0: normal grab for actor of their size
		//
		HugShrink::HugActor_Actor(giant, tiny, 1.0);
	}

	void HugShrink::Reset() {
		this->data.clear();
	}

	void HugShrink::ResetActor(Actor* actor) {
		this->data.erase(actor);
	}

	void HugShrink::Release(Actor* giant) {
		HugShrink::GetSingleton().data.erase(giant);
	}

	void HugShrink::CallRelease(Actor* giant) {
		auto huggedActor = HugShrink::GetHuggiesActor(giant);
		if (!huggedActor) {
			return;
		}
		std::string_view message = std::format("{} was saved from your hugs", huggedActor->GetDisplayFullName());
		float sizedifference = get_visual_scale(giant)/get_visual_scale(huggedActor);
		if (giant->formID == 0x14) {
			shake_camera(giant, 0.25 * sizedifference, 0.35);
		} else {
			Rumble::Once("HugGrab", giant, sizedifference * 12, 0.15);
		}
		Notify(message);
		AbortAnimation(giant, huggedActor);
	}

	TESObjectREFR* HugShrink::GetHuggiesObj(Actor* giant) {
		try {
			auto& me = HugShrink::GetSingleton();
			return me.data.at(giant).tiny;
		} catch (std::out_of_range e) {
			return nullptr;
		}

	}
	Actor* HugShrink::GetHuggiesActor(Actor* giant) {
		auto obj = HugShrink::GetHuggiesObj(giant);
		Actor* actor = skyrim_cast<Actor*>(obj);
		if (actor) {
			return actor;
		} else {
			return nullptr;
		}
	}

	void HugShrink::RegisterEvents() {
		InputManager::RegisterInputEvent("HugAttempt", HugAttemptEvent);
		InputManager::RegisterInputEvent("HugRelease", HugReleaseEvent);
		InputManager::RegisterInputEvent("HugShrink", HugShrinkEvent);
		InputManager::RegisterInputEvent("HugCrush", HugCrushEvent);

		AnimationManager::RegisterEvent("GTS_Hug_Grab", "Hugs", GTS_Hug_Grab);
		AnimationManager::RegisterEvent("GTS_Hug_Grow", "Hugs", GTS_Hug_Grow);
		AnimationManager::RegisterEvent("GTS_Hug_Moan", "Hugs", GTS_Hug_Moan);
		AnimationManager::RegisterEvent("GTS_Hug_Moan_End", "Hugs", GTS_Hug_Moan_End);
		AnimationManager::RegisterEvent("GTS_Hug_PullBack", "Hugs", GTS_Hug_PullBack);
		AnimationManager::RegisterEvent("GTS_Hug_FacialOn", "Hugs", GTS_Hug_FacialOn);
		AnimationManager::RegisterEvent("GTS_Hug_FacialOff", "Hugs", GTS_Hug_FacialOff);
		AnimationManager::RegisterEvent("GTS_Hug_CrushTiny", "Hugs", GTS_Hug_CrushTiny);
		AnimationManager::RegisterEvent("GTS_Hug_ShrinkPulse", "Hugs", GTS_Hug_ShrinkPulse);
		AnimationManager::RegisterEvent("GTS_Hug_RunShrinkTask", "Hugs", GTS_Hug_RunShrinkTask);
		AnimationManager::RegisterEvent("GTS_Hug_StopShrinkTask", "Hugs", GTS_Hug_StopShrinkTask);

		AnimationManager::RegisterEvent("GTSBEH_HugAbsorbAtk", "Hugs", GTSBEH_HugAbsorbAtk);
	}

	void HugShrink::RegisterTriggers() {
		AnimationManager::RegisterTrigger("Huggies_Try", "Hugs", "GTSBEH_HugAbsorbStart_A");
		AnimationManager::RegisterTrigger("Huggies_Try_Victim", "Hugs", "GTSBEH_HugAbsorbStart_V");
		AnimationManager::RegisterTrigger("Huggies_Shrink", "Hugs", "GTSBEH_HugAbsorbAtk");
		AnimationManager::RegisterTrigger("Huggies_Shrink_Victim", "Hugs", "GTSBEH_HugAbsorbAtk_V");
		AnimationManager::RegisterTrigger("Huggies_Spare", "Hugs", "GTSBEH_HugAbsorbExitLoop");
		AnimationManager::RegisterTrigger("Huggies_Cancel", "Hugs", "GTSBEH_PairedAbort");
		AnimationManager::RegisterTrigger("Huggies_HugCrush", "Hugs", "GTSBEH_HugCrushStart_A");
		AnimationManager::RegisterTrigger("Huggies_HugCrush_Victim", "Hugs", "GTSBEH_HugCrushStart_V");
	}

	HugShrinkData::HugShrinkData(TESObjectREFR* tiny, float strength) : tiny(tiny), strength(strength) {
	}
}
