#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/damage/AccurateDamage.hpp"
#include "managers/damage/LaunchActor.hpp"
#include "managers/animation/Growth.hpp"
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
	void GTSGrowth_Enter(AnimationEventData& data) {
    }
    void GTSGrowth_SpurtStart(AnimationEventData& data) {
    }
    void GTSGrowth_SpurtSlowdownPoint(AnimationEventData& data) {
    }
    void GTSGrowth_SpurtStop(AnimationEventData& data) {
    }
    void GTSGrowth_Exit(AnimationEventData& data) {
    }
}

namespace Gts
{
	void AnimationGrowth::RegisterEvents() {
		AnimationManager::RegisterEvent("GTSGrowth_Enter", "Growth", GTSGrowth_Enter);
        AnimationManager::RegisterEvent("GTSGrowth_SpurtStart", "Growth", GTSGrowth_SpurtStart);
        AnimationManager::RegisterEvent("GTSGrowth_SpurtSlowdownPoint", "Growth", GTSGrowth_SpurtSlowdownPoint);
        AnimationManager::RegisterEvent("GTSGrowth_SpurtStop", "Growth", GTSGrowth_SpurtStop);
        AnimationManager::RegisterEvent("GTSGrowth_Exit", "Growth", GTSGrowth_Exit);
	}

	void AnimationGrowth::RegisterTriggers() {
		AnimationManager::RegisterTrigger("TriggerGrowth", "Growth", "GTSBeh_Grow_Trigger");
	}
}