#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/damage/AccurateDamage.hpp"
#include "managers/animation/FootGrind.hpp"
#include "managers/damage/LaunchActor.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/InputManager.hpp"
#include "managers/CrushManager.hpp"
#include "managers/explosion.hpp"
#include "managers/footstep.hpp"
#include "managers/highheel.hpp"
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
    const std::string_view RNode = "NPC R Foot [Rft ]";
	const std::string_view LNode = "NPC L Foot [Lft ]";

    void ApplyDustRing(Actor* giant, FootEvent kind, std::string_view node, float mult) {
        auto& explosion = ExplosionManager::GetSingleton();
		Impact impact_data = Impact {
			.actor = giant,
			.kind = kind,
			.scale = get_visual_scale(giant) * mult,
			.nodes = find_node(giant, node),
		};
		explosion.OnImpact(impact_data); // Play explosion
    }

    void ApplyDamageOverTime_Left(Actor* giant) {
		auto gianthandle = giant->CreateRefHandle();
		std::string name = std::format("FGD_L_{}", giant->formID);
		TaskManager::Run(name, [=](auto& progressData) {
			if (!gianthandle) {
				return false;
			}
			auto giantref = gianthandle.get().get();
            Rumble::Once("FootGrindL", giantref, 1.0, 0.025, LNode);
			DoDamageEffect(giantref, 0.008, 1.5, 10000, 0.05, FootEvent::Left, 2.5);
			return true;
		});
	}

     void ApplyDamageOverTime_Right(Actor* giant) {
		auto gianthandle = giant->CreateRefHandle();
		std::string name = std::format("FGD_R_{}", giant->formID);
		TaskManager::Run(name, [=](auto& progressData) {
			if (!gianthandle) {
				return false;
			}
			auto giantref = gianthandle.get().get();
            Rumble::Once("FootGrindR", giantref, 1.0, 0.025, RNode);
			DoDamageEffect(giantref, 0.008, 1.5, 10000, 0.05, FootEvent::Right, 2.5);
			return true;
		});
	}

    void CancelDamageOverTime(Actor* giant) {
        std::string Right = std::format("FGD_L_{}", giant->formID);
        std::string Left = std::format("FGD_R_{}", giant->formID);
        TaskManager::Cancel(Right);
        TaskManager::Cancel(Left);
    }

    void GTSstomp_FootGrindL_Enter(AnimationEventData& data) {
        data.stage = 1;
        data.canEditAnimSpeed = true;
        data.animSpeed = 1.0;
    }

    void GTSstomp_FootGrindR_Enter(AnimationEventData& data) {
        data.stage = 1;
        data.canEditAnimSpeed = true;
        data.animSpeed = 1.0;
    }
       
    void GTSstomp_FootGrindL_MV_S(AnimationEventData& data) { // Feet starts to move: Left
        ApplyDamageOverTime_Left(&data.giant);
        ApplyDustRing(&data.giant, FootEvent::Left, LNode, 0.6);
    }

    void GTSstomp_FootGrindR_MV_S(AnimationEventData& data) { // Feet start to move: Right
        ApplyDamageOverTime_Right(&data.giant);
        ApplyDustRing(&data.giant, FootEvent::Right, RNode, 0.6);
    }

    void GTSstomp_FootGrindL_MV_E(AnimationEventData& data) { // When movement ends: Left
        CancelDamageOverTime(&data.giant);
        ApplyDustRing(&data.giant, FootEvent::Left, LNode, 0.6);
    }

    void GTSstomp_FootGrindR_MV_E(AnimationEventData& data) { // When movement ends: Right
        CancelDamageOverTime(&data.giant);
        ApplyDustRing(&data.giant, FootEvent::Right, RNode, 0.6);
    }

    void GTSstomp_FootGrindR_Impact(AnimationEventData& data) { // When foot hits the ground after lifting the leg up. R Foot
        float perk = GetPerkBonus_Basics(&data.giant);
        ApplyDustRing(&data.giant, FootEvent::Right, RNode, 1.05);
        DoFootstepSound(&data.giant, 1.0, FootEvent::Right, RNode);
        DoLaunch(&data.giant, 0.75 * perk, 1.8, 1.4, FootEvent::Right, 0.80);  // To-do: disallow Launching when actor is being grinded through Transient
        DoDamageEffect(&data.giant, 2.0, 1.70, 20, 0.15, FootEvent::Right, 1.0);
        Rumble::Once("GrindStompR", &data.giant, 1.25, 0.05, RNode);
    }

    void GTSstomp_FootGrindL_Impact(AnimationEventData& data) { // When foot hits the ground after lifting the leg up. L Foot
        float perk = GetPerkBonus_Basics(&data.giant);
        ApplyDustRing(&data.giant, FootEvent::Left, LNode, 1.05);
        DoFootstepSound(&data.giant, 1.0, FootEvent::Left, LNode);
        DoLaunch(&data.giant, 0.75 * perk, 1.8, 1.4, FootEvent::Left, 0.80);  // To-do: disallow Launching when actor is being grinded through Transient
        DoDamageEffect(&data.giant, 2.0, 1.70, 20, 0.15, FootEvent::Left, 1.0);
        Rumble::Once("GrindStompL", &data.giant, 1.25, 0.05, LNode);
    }

    void GTSstomp_FootGrindR_Exit(AnimationEventData& data) { // Remove foot from enemy: Right
        data.stage = 1;
        data.canEditAnimSpeed = false;
        data.animSpeed = 1.0;
        CancelDamageOverTime(&data.giant); 
        Cprint("FootGrindL Exit Fired.");
    }

    void GTSstomp_FootGrindL_Exit(AnimationEventData& data) { // Remove foot from enemy: Left
        data.stage = 1;
        data.canEditAnimSpeed = false;
        data.animSpeed = 1.0;
        CancelDamageOverTime(&data.giant);
        Cprint("FootGrindL Exit Fired.");
    }
}

namespace Gts
{
	void AnimationFootGrind::RegisterEvents() {
        AnimationManager::RegisterEvent("GTSstomp_FootGrindL_Enter", "Stomp", GTSstomp_FootGrindL_Enter);
        AnimationManager::RegisterEvent("GTSstomp_FootGrindR_Enter", "Stomp", GTSstomp_FootGrindR_Enter);
        AnimationManager::RegisterEvent("GTSstomp_FootGrindL_MV_S", "Stomp", GTSstomp_FootGrindL_MV_S);
        AnimationManager::RegisterEvent("GTSstomp_FootGrindR_MV_S", "Stomp", GTSstomp_FootGrindR_MV_S);
        AnimationManager::RegisterEvent("GTSstomp_FootGrindL_MV_E", "Stomp", GTSstomp_FootGrindL_MV_E);
        AnimationManager::RegisterEvent("GTSstomp_FootGrindR_MV_E", "Stomp", GTSstomp_FootGrindR_MV_E);
        AnimationManager::RegisterEvent("GTSstomp_FootGrindR_Impact", "Stomp", GTSstomp_FootGrindR_Impact);
        AnimationManager::RegisterEvent("GTSstomp_FootGrindL_Impact", "Stomp", GTSstomp_FootGrindL_Impact);
        AnimationManager::RegisterEvent("GTSstomp_FootGrindR_Exit", "Stomp", GTSstomp_FootGrindR_Exit);
        AnimationManager::RegisterEvent("GTSstomp_FootGrindL_Exit", "Stomp", GTSstomp_FootGrindL_Exit);
	}

    void AnimationFootGrind::RegisterTriggers() {
		AnimationManager::RegisterTrigger("GrindRight", "Stomp", "GTSBEH_StartFootGrindR");
        AnimationManager::RegisterTrigger("GrindLeft", "Stomp", "GTSBEH_StartFootGrindL");
	}
}