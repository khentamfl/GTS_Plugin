#include "managers/animation/Animation_ThighCrush.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/GrowthTremorManager.hpp"
#include "managers/ShrinkToNothingManager.hpp"
#include "managers/CrushManager.hpp"
#include "managers/impact.hpp"
#include "magic/effects/common.hpp"
#include "managers/GtsManager.hpp"
#include "utils/actorUtils.hpp"
#include "data/persistent.hpp"
#include "data/transient.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "data/time.hpp"
#include "events.hpp"
#include "timer.hpp"
#include "node.hpp"

using namespace RE;
using namespace Gts;
using namespace std;

namespace { 
    const std::vector<std::string_view> Triggers = { // Triggers 
        "ThighLoopEnter",               // [0]
        "ThighLoopAttack",              // [1]
        "ThighLoopExit",                // [2]
        "ThighLoopFull",                // [3] Play full anim
    };

    const std::vector<std::string_view> Behavior_ThighCrush = { // Behaviors
        "GTSBeh_TriggerSitdown",        // [0] Enter sit loop
	    "GTSBeh_StartThighCrush",       // [1] Trigger thigh crush
	    "GTSBeh_LeaveSitdown",          // [2] Exit animation
        "GTSBeh_ThighAnimationFull",    // [3] Play Full Animation without loops. Optional.
    };

    const std::vector<std::string_view> Anim_ThighCrush = { // Events
		"GTStosit", 				    // [0] Start air rumble and camera shake
		"GTSsitloopenter", 			    // [1] Sit down completed
		"GTSsitloopstart", 			    // [2] enter sit crush loop
 		"GTSsitloopend", 			    // [3] unused
		"GTSsitcrushlight_start",	    // [4] Start Spreading legs
		"GTSsitcrushlight_end", 	    // [5] Legs fully spread
		"GTSsitcrushheavy_start",	    // [6] Start Closing legs together
		"GTSsitcrushheavy_end", 	    // [7] Legs fully closed
		"GTSsitloopexit", 			    // [8] stand up, small air rumble and camera shake
		"GTSstandR", 				    // [9] feet collides with ground when standing up
		"GTSstandL",                    // [10]
		"GTSstandRS",                   // [11] Silent impact of right feet
		"GTStoexit", 				    // [12] Leave animation, disable air rumble and such
    };

    void SetThighStage(Actor* actor, float number) {
        auto transient = Transient::GetSingleton().GetActorData(actor);
		if (!transient) {
			return;
		}
        transient->ThighAnimStage = number;
    }
}

namespace Gts {
	ThighCrush& ThighCrush::GetSingleton() noexcept {
		static ThighCrush instance;
		return instance;
	}

	std::string ThighCrush::DebugName() {
		return "ThighCrush";
	}

    void ThighCrush::ActorAnimEvent(Actor* actor, const std::string_view& tag, const std::string_view& payload) {
        auto transient = Transient::GetSingleton().GetActorData(actor);
        if (transient) {
			if (tag == Anim_ThighCrush[0]) {
				transient->rumblemult = 0.7;
                SetThighStage(actor, 2.0);
			} if (tag == Anim_ThighCrush[1]) {
				transient->rumblemult = 0.3;
				transient->disablehh = true;
			} if (tag == Anim_ThighCrush[2]) {
				transient->rumblemult = 0.4;
                SetThighStage(actor, 2.0);
			} if (tag == Anim_ThighCrush[4]) {
				transient->rumblemult = 0.0;
				transient->legsspreading = 1.0;
			} if (tag == Anim_ThighCrush[5]) {
				transient->legsspreading = 0.6;
			} if (tag == Anim_ThighCrush[6]) {
				transient->legsspreading = 0.0;
				transient->legsclosing = 3.0;
			} if (tag == Anim_ThighCrush[7]) {
				transient->legsclosing = 1.5;
			} if (tag == Anim_ThighCrush[8]) {
				transient->disablehh = false;
				transient->legsclosing = 0.0;
				transient->rumblemult = 0.5;
			} if (tag == Anim_ThighCrush[9] || tag == Anim_ThighCrush[10] || tag == Anim_ThighCrush[11]) {
                float scale = get_visual_scale(actor);
				transient->rumblemult = 0.2;
				Runtime::PlaySound("lFootstepL", actor, scale * 0.20, 1.0);
			} if (tag == Anim_ThighCrush[12]) {
				transient->rumblemult = 0.0;
                SetThighStage(actor, 0.0);
			}
		}
    }

    void ThighCrush::ApplyThighCrush(Actor* actor, std::string_view condition) {
		if (!actor) {
			return;
		} 
        auto transient = Transient::GetSingleton().GetActorData(actor);
		if (!transient) {
			return;
		}
        log::info("Condition:{}, Thigh Stage: {}", condition, transient->ThighAnimStage);
		if (condition == Triggers[0] && transient->ThighAnimStage <= 1.0) {
            log::info("Trigger = 0");
			actor->NotifyAnimationGraph(Behavior_ThighCrush[0]);
			return;
		}
		if (condition == Triggers[1] && transient->ThighAnimStage == 2.0) {
            log::info("Trigger = 1");
			actor->NotifyAnimationGraph(Behavior_ThighCrush[1]);
			return;
		}
		if (condition == Triggers[2] && transient->ThighAnimStage >= 2.0) {
            log::info("Trigger = 2");
			actor->NotifyAnimationGraph(Behavior_ThighCrush[2]);
			return;
		}
        if (condition == Triggers[3]) {
            log::info("Trigger = 3");
            actor->NotifyAnimationGraph(Behavior_ThighCrush[3]);
        }
    }
}