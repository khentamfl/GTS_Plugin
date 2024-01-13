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


    bool Hugs_RestoreHealth(Actor* giantref, Actor* tinyref) {
		static Timer HeartTimer = Timer(0.5);
		float hp = GetAV(tinyref, ActorValue::kHealth);
		float maxhp = GetMaxAV(tinyref, ActorValue::kHealth);
		bool Healing = IsHugHealing(giantref);
		
		if (Healing && HeartTimer.ShouldRunFrame()) {
			NiPoint3 POS = GetHeartPosition(giantref, tinyref);
			if (POS.Length() > 0) {
				float scale = get_visual_scale(giantref);
				SpawnParticle(giantref, 3.00, "GTS/Magic/Hearts.nif", NiMatrix3(), POS, scale * 2.4, 7, nullptr);
			}
		}

		if (!Healing && hp >= maxhp) {
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
		tinyref->AsActorValueOwner()->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, ActorValue::kHealth, maxhp * 0.004 * 0.15 * TimeScale());

		if (!Healing) {
			return false;
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

			float tiny_size = get_visual_scale(tinyref);
			float gts_size = get_visual_scale(giantref);

			float sizedifference_gts = gts_size/tiny_size;
			float sizedifference_tiny = tiny_size/gts_size;
			float threshold = 3.0;
			float stamina = 0.35;
			float steal = GetHugStealRate(giantref);

			if (Runtime::HasPerkTeam(giantref, "HugCrush_Greed")) {
				stamina *= 0.75;
			}
			stamina *= Perk_GetCostReduction(giantref);

			if (giantref->formID != 0x14) {
				ShutUp(giantref); // STFU GTS
			}
			ShutUp(tinyref); // Disallow idle dialogues

			if (sizedifference_gts >= 1.08 || sizedifference_tiny >= threshold) {
				SetBeingHeld(tinyref, false);
				AbortHugAnimation(giantref, tinyref);
				if (giantref->formID == 0x14) {
					shake_camera(giantref, 0.50, 0.15);
					Notify("It's difficult to gently hug {}", tinyref->GetDisplayFullName());
				}
				return false;
			}
			DamageAV(tinyref, ActorValue::kStamina, -(0.45 * TimeScale())); // Restore Tiny stamina
			DamageAV(giantref, ActorValue::kStamina, 0.25 * stamina * TimeScale()); // Damage GTS Stamina

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