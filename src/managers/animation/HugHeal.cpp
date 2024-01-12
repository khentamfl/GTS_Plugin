#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/animation/Controllers/HugController.hpp"
#include "managers/emotions/EmotionManager.hpp"
#include "managers/ShrinkToNothingManager.hpp"
#include "managers/damage/SizeHitEffects.hpp"
#include "managers/animation/HugShrink.hpp"
#include "managers/damage/LaunchActor.hpp"
#include "managers/animation/HugHeal.hpp"
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

    void AdjustBehaviors(Actor* giant, Actor* tiny) { // blend between two anims: send value to behaviors
        float tinySize = get_visual_scale(tiny);
        float giantSize = get_visual_scale(giant);
        float size_difference_gtspov = std::clamp(giantSize/tinySize, 0.02f, 1.0f);
        float size_difference_tinypov = std::clamp(tinySize/giantSize, 0.02f, 1.0f);

		giant->SetGraphVariableFloat("GTS_SizeDifference", size_difference_tinypov); // pass Tiny / Giant size diff POV to GTS
		tiny->SetGraphVariableFloat("GTS_SizeDifference", size_difference_tinypov); // pass Tiny / Giant size diff POV to Tiny
    }

    bool Hugs_RestoreHealth(Actor* giantref, Actor* tinyref, float steal) {
		static Timer HeartTimer = Timer(0.5);
		float hp = GetAV(tinyref, ActorValue::kHealth);
		float maxhp = GetMaxAV(tinyref, ActorValue::kHealth);

		tinyref->AsActorValueOwner()->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, ActorValue::kHealth, maxhp * 0.008 * steal * TimeScale());
		
		if (HeartTimer.ShouldRunFrame()) {
			NiPoint3 POS = GetHeartPosition(giantref, tinyref);
			if (POS.Length() > 0) {
				float scale = get_visual_scale(giantref);
				SpawnParticle(giantref, 3.00, "GTS/Magic/Hearts.nif", NiMatrix3(), POS, scale * 2.4, 7, nullptr);
			}
		}


		if (IsHugCrushing(giantref) || IsHugHealing(giantref)) {
			return true; // disallow to cancel it during Hug Crush/Heal
		}

		if (hp >= maxhp) {
			AbortHugAnimation(giantref, tinyref);
			if (giantref->formID == 0x14) {
				Notify("{} health is full", tinyref->GetDisplayFullName());
			}
			return false;
		}

		if (giantref->formID == 0x14) {
			float sizedifference = get_visual_scale(giantref)/get_visual_scale(tinyref);
			shake_camera(giantref, 0.90 * sizedifference, 0.05);
		} else {
			GRumble::Once("HugSteal", giantref, get_visual_scale(giantref) * 8, 0.10);
		}
		return true;
	}

    void HealOtherTask(Actor* giant, Actor* tiny) {
		if (!giant) {
			return;
		}
		if (!tiny) {
			return;
		}
		std::string name = std::format("Huggies_Heal_{}", giant->formID);
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

			if (!IsHugHealing(giantref)) {
				Notify("Task has ended");
				return false; // end task in that case
			}

			float sizedifference = get_target_scale(giantref)/get_target_scale(tinyref);
			float threshold = 2.5;
			float stamina = 0.35;
			float steal = GetHugStealRate(giantref);

			AdjustBehaviors(giantref, tinyref);

			if (Runtime::HasPerkTeam(giantref, "HugCrush_Greed")) {
				stamina *= 0.75;
			}
			stamina *= Perk_GetCostReduction(giantref);

			if (giantref->formID != 0x14) {
				ShutUp(giantref); // STFU GTS
			}
			ShutUp(tinyref); // Disallow idle dialogues

			if (sizedifference >= threshold) {
				SetBeingHeld(tinyref, false);
				AbortHugAnimation(giantref, tinyref);
				return false;
			}
			DamageAV(tinyref, ActorValue::kStamina, -(0.45 * TimeScale())); // Restore Tiny stamina
			DamageAV(giantref, ActorValue::kStamina, 0.25 * stamina * TimeScale()); // Damage GTS Stamina
            
			if (giantref->formID == 0x14) {
				shake_camera(giantref, 0.30 * sizedifference, 0.05);
			} else {
				GRumble::Once("HugSteal", giantref, get_visual_scale(giantref) * 4, 0.10);
			}

			return Hugs_RestoreHealth(giantref, tinyref, steal);

		});
	}

    void GTS_Hug_Heal(AnimationEventData& data) {
        auto huggedActor = HugShrink::GetHuggiesActor(&data.giant);
		if (huggedActor) {
			HealOtherTask(&data.giant, huggedActor);
		}
    }
}


namespace Gts {
    void HugHeal::RegisterEvents() {
        AnimationManager::RegisterEvent("GTS_Hug_Heal", "Hugs", GTS_Hug_Heal);
    }

    void HugHeal::RegisterTriggers() {
		AnimationManager::RegisterTrigger("Huggies_Heal", "Hugs", "GTSBEH_HugHealStart_A");
		AnimationManager::RegisterTrigger("Huggies_Heal_Victim_F", "Hugs", "GTSBEH_HugHealStart_Fem_V");
		AnimationManager::RegisterTrigger("Huggies_Heal_Victim_M", "Hugs", "GTSBEH_HugHealStart_Mal_V");
	}
}