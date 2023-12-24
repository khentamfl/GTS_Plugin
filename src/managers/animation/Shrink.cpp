#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/emotions/EmotionManager.hpp"
#include "managers/damage/CollisionDamage.hpp"
#include "managers/damage/LaunchActor.hpp"
#include "managers/animation/Shrink.hpp"
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
#include "timer.hpp"
#include "node.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace {

	void SetHalfLife(Actor* actor, float value) {
		auto& Persist = Persistent::GetSingleton();
		auto actor_data = Persist.GetData(actor);
		if (actor_data) {
			actor_data->half_life = value;
		}
	}

	void CancelShrink(Actor* actor) {
		std::string name = std::format("ManualShrink_{}", actor->formID);
		TaskManager::Cancel(name);
		SetHalfLife(actor, 1.0);
	}

	void ShrinkTask(Actor* actor) {
		if (!actor) {
			return;
		}
		float Start = Time::WorldTimeElapsed();
		ActorHandle gianthandle = actor->CreateRefHandle();
		std::string name = std::format("ManualShrink_{}", actor->formID);
		SetHalfLife(actor, 0.0);
		TaskManager::Run(name, [=](auto& progressData) {
			if (!gianthandle) {
				return false;
			}
			auto caster = gianthandle.get().get();
			float timepassed = Time::WorldTimeElapsed() - Start;
			float elapsed = std::clamp(timepassed * AnimationManager::GetAnimSpeed(caster), 0.01f, 1.2f);
			float multiply = bezier_curve(elapsed, 0, 1.9, 0.6, 0, 2.0, 1.0);
			//                            ^value   x1  x2  x3  x4  i     k

			float caster_scale = get_visual_scale(caster);
			float stamina = clamp(0.05, 1.0, GetStaminaPercentage(caster));

			DamageAV(caster, ActorValue::kStamina, 0.45 * caster_scale * stamina * TimeScale() * multiply);
			Grow(caster, -(0.0080 * stamina * multiply), 0.0);

			GRumble::Once("ShrinkButton", caster, 4.0 * stamina, 0.05, "NPC Pelvis [Pelv]");
			if (elapsed >= 0.99) {
				SetHalfLife(caster, 1.0);
				return false;
			}
			return true;
		});
	}
	void GTSShrink_Enter(AnimationEventData& data) {
	}
	void GTSShrink_StartShrink(AnimationEventData& data) {
		ShrinkTask(&data.giant);
	}
	void GTSShrink_SlowShrink(AnimationEventData& data) {
	}
	void GTSShrink_StopShrink(AnimationEventData& data) {
		//CancelShrink(&data.giant);
	}
	void GTSShrink_Exit(AnimationEventData& data) {
	}
}

namespace Gts
{
	void AnimationShrink::RegisterEvents() {
		AnimationManager::RegisterEvent("GTSShrink_Enter", "Shrink", GTSShrink_Enter);
		AnimationManager::RegisterEvent("GTSShrink_StartShrink", "Shrink", GTSShrink_StartShrink);
		AnimationManager::RegisterEvent("GTSShrink_SlowShrink", "Shrink", GTSShrink_SlowShrink);
		AnimationManager::RegisterEvent("GTSShrink_StopShrink", "Shrink", GTSShrink_StopShrink);
		AnimationManager::RegisterEvent("GTSShrink_Exit", "Shrink", GTSShrink_Exit);
	}

	void AnimationShrink::RegisterTriggers() {
		AnimationManager::RegisterTrigger("TriggerShrink", "Shrink", "GTSBeh_Shrink_Trigger");
	}
}