// Animation: Strong Stomp
//  - Stages
/*
GTS_StrongStomp_Start
GTS_StrongStomp_LR_Start
GTS_StrongStomp_LL_Start
GTS_StrongStomp_LR_Middle
GTS_StrongStomp_LL_Middle
GTS_StrongStomp_LR_End
GTS_StrongStomp_LL_End
GTS_StrongStomp_ImpactR
GTS_StrongStomp_ImpactL
GTS_StrongStomp_ReturnRL_Start
GTS_StrongStomp_ReturnLL_Start
GTS_StrongStomp_ReturnRL_End
GTS_StrongStomp_ReturnLL_End
GTS_StrongStomp_End
*/

#include "managers/animation/StrongStomp.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/damage/AccurateDamage.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/InputManager.hpp"
#include "managers/CrushManager.hpp"
#include "managers/explosion.hpp"
#include "managers/footstep.hpp"
#include "utils/actorUtils.hpp"
#include "managers/Rumble.hpp"
#include "managers/tremor.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "node.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace {

    const std::vector<std::string_view> R_LEG_RUMBLE_NODES = { // used with Anim_ThighCrush
		"NPC L Foot [Lft ]",
		"NPC L Toe0 [LToe]",
		"NPC L Calf [LClf]",
		"NPC L PreRearCalf",
		"NPC L FrontThigh",
		"NPC L RearCalf [RrClf]",
	};
    const std::vector<std::string_view> L_LEG_RUMBLE_NODES = { // used with Anim_ThighCrush
		"NPC R Foot [Rft ]",
		"NPC R Toe0 [RToe]",
		"NPC R Calf [RClf]",
		"NPC R PreRearCalf",
		"NPC R FrontThigh",
		"NPC R RearCalf [RrClf]",
	};

	const std::string_view RNode = "NPC R Foot [Rft ]";
	const std::string_view LNode = "NPC L Foot [Lft ]";

    void StartLegRumble(std::string_view tag, Actor& actor, float power, float halflife, std::string_view type) {
        if (type == "Left") {
		    for (auto& node_name: L_LEG_RUMBLE_NODES) {
			    std::string rumbleName = std::format("{}{}", tag, node_name);
			    Rumble::Start(rumbleName, &actor, power,  halflife, node_name);
		    }
        } else if (type == "Right") {
            for (auto& node_name: R_LEG_RUMBLE_NODES) {
			    std::string rumbleName = std::format("{}{}", tag, node_name);
			    Rumble::Start(rumbleName, &actor, power,  halflife, node_name);
		    }
        }
	}

	void StopLegRumble(std::string_view tag, Actor& actor, std::string_view type) {
        if (type == "Left") {
		    for (auto& node_name: L_LEG_RUMBLE_NODES) {
			    std::string rumbleName = std::format("{}{}", tag, node_name);
			    Rumble::Stop(rumbleName, &actor);
		    }
        } else if (type == "Right") {
            for (auto& node_name: R_LEG_RUMBLE_NODES) {
			    std::string rumbleName = std::format("{}{}", tag, node_name);
			    Rumble::Stop(rumbleName, &actor);
		    }
        }
	}

	void GTS_StrongStomp_Start(AnimationEventData& data) {
		data.stage = 1;
		data.animSpeed = 1.15;
		Rumble::Start("StompR", &data.giant, 0.35, 0.15, RNode);
	}

	void GTS_StrongStomp_LR_Start(AnimationEventData& data) {
		data.stage = 1;
		data.canEditAnimSpeed = true;
		StartLegRumble("StrongStompR", data.giant, 0.6, 0.10, "Right");
	}

    void GTS_StrongStomp_LL_Start(AnimationEventData& data) {
		data.stage = 1;
		data.canEditAnimSpeed = true;
		StartLegRumble("StrongStompL", data.giant, 0.6, 0.10, "Left");
	}

    void GTS_StrongStomp_LR_Middle(AnimationEventData& data) {
	}
    void GTS_StrongStomp_LL_Middle(AnimationEventData& data) {
	}
    void GTS_StrongStomp_LR_End(AnimationEventData& data) {
        data.canEditAnimSpeed = false;
        StopLegRumble("StrongStompR", data.giant, "Right");
    }
    void GTS_StrongStomp_LL_End(AnimationEventData& data) {
        data.canEditAnimSpeed = false;
        StopLegRumble("StrongStompL", data.giant, "Left");
    }

    void GTS_StrongStomp_ImpactR(AnimationEventData& data) {
        Rumble::Once("HeavyStompR", &data.giant, 3.45 * data.animSpeed, 0.10, RNode);
        DoDamageEffect(&data.giant, 6.0 * data.animSpeed, 2.0 * data.animSpeed, 5, 0.60);
		DoSizeEffect(&data.giant, 3.10 * data.animSpeed, FootEvent::Right, RNode);
        data.animSpeed = 1.0;
    }
    void GTS_StrongStomp_ImpactL(AnimationEventData& data) {
        Rumble::Once("HeavyStompL", &data.giant, 3.45 * data.animSpeed, 0.10, LNode);
        DoDamageEffect(&data.giant, 6.0 * data.animSpeed, 2.0 * data.animSpeed, 5, 0.60);
		DoSizeEffect(&data.giant, 3.10 * data.animSpeed, FootEvent::Right, LNode);
        data.animSpeed = 1.0;
    }
    void GTS_StrongStomp_ReturnRL_Start(AnimationEventData& data) {
        StartLegRumble("StrongStompR", data.giant, 0.4, 0.10, "Right");
    }
    void GTS_StrongStomp_ReturnLL_Start(AnimationEventData& data) {
        StartLegRumble("StrongStompL", data.giant, 0.4, 0.10, "Left");
    }
    void GTS_StrongStomp_ReturnRL_End(AnimationEventData& data) {
        StopLegRumble("StrongStompR", data.giant, "Right");
    }
    void GTS_StrongStomp_ReturnLL_End(AnimationEventData& data) {
        StopLegRumble("StrongStompL", data.giant, "Left");
    }
    void GTS_StrongStomp_End(AnimationEventData& data) {
    }


	void GTS_Next(AnimationEventData& data) {
		Rumble::Stop("StompR", &data.giant);
	}

	void GTSBEH_Exit(AnimationEventData& data) {
		Rumble::Stop("StompR", &data.giant);
	}

	void RightStrongStompEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		if (Runtime::HasPerk(player, "DestructionBasics") && GetAV(player, ActorValue::kStamina) > 75) {
			AnimationManager::StartAnim("StrongStompRight", player);
            DamageAV(player, ActorValue::kStamina, 75);
		}
	}

	void LeftStrongStompEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		if (Runtime::HasPerk(player, "DestructionBasics") && GetAV(player, ActorValue::kStamina) > 75) {
			AnimationManager::StartAnim("StrongStompLeft", player);
            DamageAV(player, ActorValue::kStamina, 75);
		}
	}
}

namespace Gts
{
	void AnimationStrongStomp::RegisterEvents() {
        AnimationManager::RegisterEvent("GTS_StrongStomp_Start", "StrongStomp", GTS_StrongStomp_Start);
        AnimationManager::RegisterEvent("GTS_StrongStomp_LR_Start", "StrongStomp", GTS_StrongStomp_LR_Start);
        AnimationManager::RegisterEvent("GTS_StrongStomp_LL_Start", "StrongStomp", GTS_StrongStomp_LL_Start);
        AnimationManager::RegisterEvent("GTS_StrongStomp_LR_Middle", "StrongStomp", GTS_StrongStomp_LR_Middle);
        AnimationManager::RegisterEvent("GTS_StrongStomp_LL_Middle", "StrongStomp", GTS_StrongStomp_LL_Middle);
        AnimationManager::RegisterEvent("GTS_StrongStomp_LR_End", "StrongStomp", GTS_StrongStomp_LR_End);
        AnimationManager::RegisterEvent("GTS_StrongStomp_LL_End", "StrongStomp", GTS_StrongStomp_LL_End);
        AnimationManager::RegisterEvent("GTS_StrongStomp_ImpactR", "StrongStomp", GTS_StrongStomp_ImpactR);
        AnimationManager::RegisterEvent("GTS_StrongStomp_ImpactL", "StrongStomp", GTS_StrongStomp_ImpactL);
        AnimationManager::RegisterEvent("GTS_StrongStomp_ReturnRL_Start", "StrongStomp", GTS_StrongStomp_ReturnRL_Start);
        AnimationManager::RegisterEvent("GTS_StrongStomp_ReturnLL_Start", "StrongStomp", GTS_StrongStomp_ReturnLL_Start);
        AnimationManager::RegisterEvent("GTS_StrongStomp_ReturnRL_End", "StrongStomp", GTS_StrongStomp_ReturnRL_End);
        AnimationManager::RegisterEvent("GTS_StrongStomp_ReturnLL_End", "StrongStomp", GTS_StrongStomp_ReturnLL_End);
        AnimationManager::RegisterEvent("GTS_StrongStomp_End", "StrongStomp", GTS_StrongStomp_End);
		AnimationManager::RegisterEvent("GTS_Next", "StrongStomp", GTS_Next);
		AnimationManager::RegisterEvent("GTSBEH_Exit", "StrongStomp", GTSBEH_Exit);

		InputManager::RegisterInputEvent("RightStomp_Strong", RightStrongStompEvent);
		InputManager::RegisterInputEvent("LeftStomp_Strong", LeftStrongStompEvent);
	}

	void AnimationStrongStomp::RegisterTriggers() {
		AnimationManager::RegisterTrigger("StrongStompRight", "Stomp", "GTSBeh_StrongStomp_StartRight");
		AnimationManager::RegisterTrigger("StrongStompLeft", "Stomp", "GtsModStompAnimLeft");
	}
}
