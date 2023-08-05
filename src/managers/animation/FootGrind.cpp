
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
			DoDamageEffect(giantref, 0.025, 1.4, 100, 0.20, FootEvent::Left, 1.0);
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
			DoDamageEffect(giantref, 0.025, 1.4, 100, 0.20, FootEvent::Right, 1.0);
			return true;
		});
	}

    void CancelDamageOverTime(Actor* giant) {
        std::string Right = std::format("FGD_L_{}", giant->formID);
        std::string Left = std::format("FGD_R_{}", giant->formID);
        TaskManager::Cancel(Right);
        TaskManager::Cancel(Left);
    }
       
    void GTSstomp_FootGrindL_MV_S(AnimationEventData& data) { // Feet starts to move: Left
        ApplyDamageOverTime_Left(&data.giant);
        ApplyDustRing(&data.giant, FootEvent::Left, LNode, 0.8);
    }

    void GTSstomp_FootGrindR_MV_S(AnimationEventData& data) { // Feet start to move: Right
        ApplyDamageOverTime_Right(&data.giant);
        ApplyDustRing(&data.giant, FootEvent::Right, RNode, 0.8);
    }

    void GTSstomp_FootGrindL_MV_E(AnimationEventData& data) { // When movement ends: Left
        CancelDamageOverTime(&data.giant);
        ApplyDustRing(&data.giant, FootEvent::Left, LNode, 0.8);
    }

    void GTSstomp_FootGrindR_MV_E(AnimationEventData& data) { // When movement ends: Right
        CancelDamageOverTime(&data.giant);
        ApplyDustRing(&data.giant, FootEvent::Right, RNode, 0.8);
    }
}

namespace Gts
{
	void AnimationFootGrind::RegisterEvents() {
        AnimationManager::RegisterEvent("GTSstomp_FootGrindL_MV_S", "FootGrind", GTSstomp_FootGrindL_MV_S);
        AnimationManager::RegisterEvent("GTSstomp_FootGrindR_MV_S", "FootGrind", GTSstomp_FootGrindR_MV_S);
        AnimationManager::RegisterEvent("GTSstomp_FootGrindL_MV_E", "FootGrind", GTSstomp_FootGrindL_MV_E);
        AnimationManager::RegisterEvent("GTSstomp_FootGrindR_MV_E", "FootGrind", GTSstomp_FootGrindR_MV_E);
	}

    void AnimationFootGrind::RegisterTriggers() {
		AnimationManager::RegisterTrigger("GrindRight", "FootGrind", "GTSBEH_StartFootGrindR");
        AnimationManager::RegisterTrigger("GrindLeft", "FootGrind", "GTSBEH_StartFootGrindL");
	}
}