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

    void AdjustBehaviors(Actor* giant, Actor* tiny) { // blend between two anims: send value to behaviors
        float tinySize = get_visual_scale(tiny);
        float giantSize = get_visual_scale(giant);
        float size_difference_gtspov = giantSize/tinySize;
        float size_difference_tinypov = tinySize/giantSize;
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
		const float duration = 4.0;
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
			float threshold = 2.5;
			float stamina = 0.35;
			float steal = GetStealRate(giantref);
			if (Runtime::HasPerkTeam(giantref, "HugCrush_Greed")) {
				stamina *= 0.75;
			}
			if (Runtime::HasPerkTeam(giantref, "SkilledGTS")) {
				float level = std::clamp(GetGtsSkillLevel() * 0.0035f, 0.0f, 0.35f);
				stamina -= level;
			}

			if (giantref->formID != 0x14) {
				ShutUp(giantref); // STFU GTS
			}
			ShutUp(tinyref); // Disallow idle dialogues

			if (sizedifference >= threshold) {
				SetBeingHeld(tinyref, false);
				AbortHugAnimation(giantref, tinyref);
				return false;
			}
			DamageAV(tinyref, ActorValue::kStamina, (0.30 * TimeScale())); // Drain Stamina
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
        auto huggedActor = HugShrink::GetHuggiesActor(giant);
		if (huggedActor) {
			HealOtherTask(&data.giant, huggedActor);
		}
    }

    void HugHeal::RegisterEvents() {
        AnimationManager::RegisterEvent("GTS_Hug_Heal", "Hugs", GTS_Hug_Heal);
    }

    void HugHeal::RegisterTriggers() {
		AnimationManager::RegisterTrigger("Huggies_Heal", "Hugs", "GTSBEH_HugHealStart_A");
		AnimationManager::RegisterTrigger("Huggies_Heal_Victim_F", "Hugs", "GTSBEH_HugHealStart_Fem_V");
		AnimationManager::RegisterTrigger("Huggies_Heal_Victim_M", "Hugs", "GTSBEH_HugHealStart_Mal_V");
	}
}