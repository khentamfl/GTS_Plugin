#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/damage/AccurateDamage.hpp"
#include "managers/damage/LaunchActor.hpp"
#include "managers/animation/Growth.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/InputManager.hpp"
#include "managers/CrushManager.hpp"
#include "magic/effects/common.hpp"
#include "managers/explosion.hpp"
#include "managers/footstep.hpp"
#include "utils/actorUtils.hpp"
#include "managers/Rumble.hpp"
#include "data/persistent.hpp"
#include "managers/tremor.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "data/time.hpp"
#include "node.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace {
    void CancelGrowth(Actor* actor) {
        std::string name = std::format("ManualGrowth_{}", actor->formID);
        TaskManager::Cancel(name);
        SetHalfLife(actor, 1.0);
    }

    void SetHalfLife(Actor* actor, float value) {
        auto& Persist = Persistent::GetSingleton();
        auto actor_data = Persist.GetData(actor);
        if (actor_data) {
			actor_data->half_life = value; 
		}
    }

    void GrowthTask(Actor* actor) {
        if (!actor) {
            return;
        }
        SetHalfLife(actor, 0.20);
		float Start = Time::WorldTimeElapsed();
		ActorHandle gianthandle = actor->CreateRefHandle();
		std::string name = std::format("ManualGrowth_{}", actor->formID);
		TaskManager::Run(name, [=](auto& progressData) {
			if (!gianthandle) {
				return false;
			}
			auto caster = gianthandle.get().get();

			float timeelapsed = std::clamp(Time::WorldTimeElapsed() - Start, 0.01f, 9999f);
            timeelapsed /= AnimationManager::GetAnimSpeed(caster);
			float multiply = bezier_curve(timeelapsed, 0, 0.9, 1, 1, 2);
			
			float caster_scale = get_visual_scale(caster);
			float stamina = clamp(0.05, 1.0, GetStaminaPercentage(caster));

			DamageAV(caster, ActorValue::kStamina, 0.45 * (caster_scale * 0.5 + 0.5) * stamina * TimeScale() * multiply);
			Grow(caster, 0.0090 * stamina * multiply, 0.0);

			Rumble::Once("GrowButton", caster, 1.0, 0.05);
			return true;
		});
	}

	void GTSGrowth_Enter(AnimationEventData& data) {
    }
    void GTSGrowth_SpurtStart(AnimationEventData& data) {
        GrowthTask(&data.giant);
    }
    void GTSGrowth_SpurtSlowdownPoint(AnimationEventData& data) {
    }
    void GTSGrowth_SpurtStop(AnimationEventData& data) {
        CancelGrowth(&data.giant);
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