#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/animation/Utils/CrawlUtils.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/damage/AccurateDamage.hpp"
#include "managers/animation/BoobCrush.hpp"
#include "managers/damage/LaunchActor.hpp"
#include "managers/animation/Grab.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/InputManager.hpp"
#include "managers/CrushManager.hpp"
#include "managers/explosion.hpp"
#include "managers/footstep.hpp"
#include "managers/highheel.hpp"
#include "utils/actorUtils.hpp"
#include "data/persistent.hpp"
#include "managers/Rumble.hpp"
#include "managers/tremor.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "data/time.hpp"
#include "node.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;
using namespace Gts;

/*
GTS_BoobCrush_Smile_On
GTS_BoobCrush_Smile_Off
GTS_BoobCrush_TrackBody        (Enables camera tracking)
GTS_BoobCrush_UnTrackBody  (Disables it)
GTS_BoobCrush_BreastImpact  (Damage everything under breasts, on impact)
GTS_BoobCrush_DOT_Start       (When we want to deal damage over time when we do long idle with swaying legs)
GTS_BoobCrush_DOT_End          
GTS_BoobCrush_Grow_Start
GTS_BoobCrush_Grow_Stop


*/

namespace {

    const std::vector<std::string_view> ALL_RUMBLE_NODES = { // used for body rumble
		"NPC COM [COM ]",
		"NPC L Foot [Lft ]",
		"NPC R Foot [Rft ]",
		"NPC L Toe0 [LToe]",
		"NPC R Toe0 [RToe]",
		"NPC L Calf [LClf]",
		"NPC R Calf [RClf]",
		"NPC L PreRearCalf",
		"NPC R PreRearCalf",
		"NPC L FrontThigh",
		"NPC R FrontThigh",
		"NPC R RearCalf [RrClf]",
		"NPC L RearCalf [RrClf]",
        "NPC L UpperarmTwist1 [LUt1]",
		"NPC L UpperarmTwist2 [LUt2]",
		"NPC L Forearm [LLar]",
		"NPC L ForearmTwist2 [LLt2]",
		"NPC L ForearmTwist1 [LLt1]",
		"NPC L Hand [LHnd]",
        "NPC R UpperarmTwist1 [RUt1]",
		"NPC R UpperarmTwist2 [RUt2]",
		"NPC R Forearm [RLar]",
		"NPC R ForearmTwist2 [RLt2]",
		"NPC R ForearmTwist1 [RLt1]",
		"NPC R Hand [RHnd]",
	};

    const std::string_view RNode = "NPC R Foot [Rft ]";
	const std::string_view LNode = "NPC L Foot [Lft ]";

    void StartRumble(std::string_view tag, Actor& actor, float power, float halflife) {
		for (auto& node_name: ALL_RUMBLE_NODES) {
			std::string rumbleName = std::format("ButtCrush_{}{}", tag, node_name);
		    Rumble::Start(rumbleName, &actor, power, halflife, node_name);
		}
	}

    void StopRumble(std::string_view tag, Actor& actor) {
		for (auto& node_name: ALL_RUMBLE_NODES) {
			std::string rumbleName = std::format("ButtCrush_{}{}", tag, node_name);
			Rumble::Stop(rumbleName, &actor);
		}
	}
}

namespace Gts
{
	void AnimationBoobCrush::RegisterEvents() {
	}

    void AnimationBoobCrush::RegisterTriggers() {
	}
}