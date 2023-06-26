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
		auto huggedActor = HugShrink::GetHuggiesActor(giant);
		AdjustFacialExpression(giant, 0, 0.75, "phenome");
		Runtime::PlaySoundAtNode("MoanSound", giant, 1.0, 1.0, "NPC Head [Head]");
		if (!huggedActor) {
			return;
		}
	}

	void GTSBEH_HugAbsorbAtk(AnimationEventData& data) {
		auto giant = &data.giant;
	}

  // Cancel all the things
  void AbortAnimation(Actor* giant, Actor* tiny) {
    AnimationManager::StartAnim("Huggies_Spare", giant);
	AdjustFacialExpression(giant, 0, 0.0, "phenome");
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

	void HugShrinkEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		auto huggedActor = HugShrink::GetHuggiesActor(player);
		if (!huggedActor) {
			return;
		}
		AnimationManager::StartAnim("Huggies_Shrink", player);
		AnimationManager::StartAnim("Huggies_Shrink_Victim", huggedActor); /// Trigger GTS_Beh that starts something
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
			if (sizedifference >= 4.0) {
				SetBeingHeld(tinyref, false);
				AbortAnimation(giantref, tinyref);
				return false;
			}
			DamageAV(tinyref, ActorValue::kStamina, 1.00 * TimeScale()); // Drain Stamina
			DamageAV(giantref, ActorValue::kStamina, -0.33 * TimeScale()); // Restore GTS Stamina
			shake_camera(giantref, 0.50 * sizedifference, 0.05);
			ShrinkActor(tinyref, 0, 0.0015);
			Grow(giantref, 0.0, 0.0003);
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


			// Exit on death
			float sizedifference = get_visual_scale(giantref)/get_visual_scale(tinyref);
			if (!FaceOpposite(giantref, tinyref)) {
				// If face towards fails then actor is invalid
				return false;
			}

			GrabStaminaDrain(giantref, tinyref, sizedifference * 2.6);

			DamageAV(giantref, ActorValue::kStamina, 0.50 * TimeScale()); // Drain Stamina

			float stamina = GetAV(giantref, ActorValue::kStamina);
			if (giantref->IsDead() || tinyref->IsDead() || stamina <= 2.0 || sizedifference >= 4.0 || !HugShrink::GetHuggiesActor(giantref)) {
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

		AnimationManager::RegisterEvent("GTS_Hug_Grab", "Hugs", GTS_Hug_Grab);
		AnimationManager::RegisterEvent("GTS_Hug_Grow", "Hugs", GTS_Hug_Grow);
		AnimationManager::RegisterEvent("GTS_Hug_Moan", "Hugs", GTS_Hug_Moan);
		AnimationManager::RegisterEvent("GTSBEH_HugAbsorbAtk", "Hugs", GTSBEH_HugAbsorbAtk);
	}

	void HugShrink::RegisterTriggers() {
		AnimationManager::RegisterTrigger("Huggies_Try", "Hugs", "GTSBEH_HugAbsorbStart_A");
		AnimationManager::RegisterTrigger("Huggies_Try_Victim", "Hugs", "GTSBEH_HugAbsorbStart_V");
		AnimationManager::RegisterTrigger("Huggies_Shrink", "Hugs", "GTSBEH_HugAbsorbAtk");
		AnimationManager::RegisterTrigger("Huggies_Shrink_Victim", "Hugs", "GTSBEH_HugAbsorbAtk_V");
		AnimationManager::RegisterTrigger("Huggies_Spare", "Hugs", "GTSBEH_HugAbsorbExitLoop");
		AnimationManager::RegisterTrigger("Huggies_Cancel", "Hugs", "GTSBEH_PairedAbort");
	}

	HugShrinkData::HugShrinkData(TESObjectREFR* tiny, float strength) : tiny(tiny), strength(strength) {
	}
}
