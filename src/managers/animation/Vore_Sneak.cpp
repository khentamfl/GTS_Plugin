
#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/animation/Utils/CrawlUtils.hpp"
#include "managers/emotions/EmotionManager.hpp"
#include "managers/animation/Vore_Sneak.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/ai/aifunctions.hpp"
#include "managers/CrushManager.hpp"
#include "utils/papyrusUtils.hpp"
#include "utils/actorUtils.hpp"
#include "data/persistent.hpp"
#include "managers/explosion.hpp"
#include "managers/footstep.hpp"
#include "managers/Rumble.hpp"
#include "managers/tremor.hpp"
#include "data/transient.hpp"
#include "managers/vore.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "node.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace {
	bool IsVoring(Actor* giant) {
		bool Voring;
		giant->GetGraphVariableBool("GTS_IsVoring", Voring);
		return Voring;
	}

    void Task_HighHeel_SyncVoreAnim(Actor* giant, bool cancel) {
		// Purpose of this task is to blend between 2 animations based on value.
		// The problem: hand that grabs the tiny is becomming offset if we equip High Heels
		// This task fixes that (by, again, blending with anim that has hand placed lower).
		std::string name = std::format("Vore_AdjustHH_{}", giant->formID);
		if (!cancel) {
			ActorHandle gianthandle = giant->CreateRefHandle();
			TaskManager::Run(name, [=](auto& progressData) {
				if (!gianthandle) {
					return false;
				}
				Actor* giantref = gianthandle.get().get();

				float hh_value = HighHeelManager::GetBaseHHOffset(giantref)[2]/100;
				float hh_offset = std::clamp(hh_value * 4.5f, 0.0f, 1.0f); // reach max HH at 0.22 offset (highest i've seen)
			
				giantref->SetGraphVariableFloat("GTS_HHoffset", hh_offset);
				// make behaviors read the value to blend between anims

				if (!IsVoring(giantref)) {
					log::info("! Is voring, task cancelled");
					return false; // just a fail-safe to cancel the task if we're outside of Vore anim
				}
				
				return true;
			});
		} else {
			TaskManager::Cancel(name);
		}
    }

    void GTS_Sneak_Vore_Start(AnimationEventData& data) {
        auto giant = &data.giant;
		auto& VoreData = Vore::GetSingleton().GetVoreData(giant);
		VoreData.AllowToBeVored(false);
		for (auto& tiny: VoreData.GetVories()) {
			AllowToBeCrushed(tiny, false);
			DisableCollisions(tiny, giant);
            SetBeingHeld(tiny, true);
		}

		Task_HighHeel_SyncVoreAnim(giant, false);
    }
    void GTS_Sneak_Vore_Grab(AnimationEventData& data) {
		auto& VoreData = Vore::GetSingleton().GetVoreData(&data.giant);
		for (auto& tiny: VoreData.GetVories()) {
			if (!Vore_ShouldAttachToRHand(&data.giant, tiny)) {
				VoreData.GrabAll();
			}
			tiny->NotifyAnimationGraph("JumpFall");
			Attacked(tiny, &data.giant);
		}
		if (IsTransferingTiny(&data.giant)) {
			ManageCamera(&data.giant, true, 4.0);
		} else {
			ManageCamera(&data.giant, true, 2.0);
		}
    }
    void GTS_Sneak_Vore_OpenMouth(AnimationEventData& data) {
        AdjustFacialExpression(&data.giant, 0, 1.0, 0.32, 0.72, "phenome"); // Start opening mouth
		AdjustFacialExpression(&data.giant, 1, 0.5, 0.32, 0.72, "phenome"); // Open it wider
		AdjustFacialExpression(&data.giant, 0, 0.8, 0.32, 0.72, "modifier"); // blink L
		AdjustFacialExpression(&data.giant, 1, 0.8, 0.32, 0.72, "modifier"); // blink R
		Cprint("OpenMouth fired");
    }
    void GTS_Sneak_Vore_Swallow(AnimationEventData& data) {
        Actor* giant = &data.giant;

        auto& VoreData = Vore::GetSingleton().GetVoreData(giant);
		for (auto& tiny: VoreData.GetVories()) {
			AllowToBeCrushed(tiny, true);
			if (tiny->formID == 0x14) {
				PlayerCamera::GetSingleton()->cameraTarget = giant->CreateRefHandle();
			}
			if (AllowDevourment()) {
				CallDevourment(giant, tiny);
				SetBeingHeld(tiny, false);
				VoreData.AllowToBeVored(true);
			} else {
				VoreData.Swallow();
				tiny->SetAlpha(0.0);
				Runtime::PlaySoundAtNode("VoreSwallow", giant, 1.0, 1.0, "NPC Head [Head]"); // Play sound
			}
		}
    }

    void GTS_Sneak_Vore_CloseMouth(AnimationEventData& data) {
		AdjustFacialExpression(&data.giant, 0, 0.0, 0.32, 0.72, "phenome"); // Start opening mouth
		AdjustFacialExpression(&data.giant, 1, 0.0, 0.32, 0.72, "phenome"); // Open it wider
		AdjustFacialExpression(&data.giant, 0, 0.0, 0.32, 0.72, "modifier"); // blink L
		AdjustFacialExpression(&data.giant, 1, 0.0, 0.32, 0.72, "modifier"); // blink R
    }

    void GTS_Sneak_Vore_KillAll(AnimationEventData& data) {
        auto giant = &data.giant;
		auto& VoreData = Vore::GetSingleton().GetVoreData(giant);
		for (auto& tiny: VoreData.GetVories()) {
			if (tiny) {
				AllowToBeCrushed(tiny, true);
				EnableCollisions(tiny);
			}
		}
		VoreData.AllowToBeVored(true);
		VoreData.KillAll();
		VoreData.ReleaseAll();

		ManageCamera(giant, false, 4.0);
		ManageCamera(giant, false, 2.0);
    }

    void GTS_Sneak_Vore_SmileOn(AnimationEventData& data) {
        AdjustFacialExpression(&data.giant, 2, 1.0, 0.32, 0.72, "expression");
		AdjustFacialExpression(&data.giant, 3, 0.8, 0.32, 0.72, "phenome");
		Cprint("SmileON fired");
    }
    void GTS_Sneak_Vore_SmileOff(AnimationEventData& data) {
        AdjustFacialExpression(&data.giant, 2, 0.0, 0.32, 0.72, "expression");
		AdjustFacialExpression(&data.giant, 3, 0.0, 0.32, 0.72, "phenome");

		Task_HighHeel_SyncVoreAnim(&data.giant, true); // cancel the task

		Cprint("SmileOFF fired");
    }
}

namespace Gts
{
	void Animation_VoreSneak::RegisterEvents() { 
		AnimationManager::RegisterEvent("GTS_Sneak_Vore_Grab", "SneakVore", GTS_Sneak_Vore_Grab);
        AnimationManager::RegisterEvent("GTS_Sneak_Vore_OpenMouth", "SneakVore", GTS_Sneak_Vore_OpenMouth);
        AnimationManager::RegisterEvent("GTS_Sneak_Vore_Swallow", "SneakVore", GTS_Sneak_Vore_Swallow);
        AnimationManager::RegisterEvent("GTS_Sneak_Vore_CloseMouth", "SneakVore", GTS_Sneak_Vore_CloseMouth);
        AnimationManager::RegisterEvent("GTS_Sneak_Vore_KillAll", "SneakVore", GTS_Sneak_Vore_KillAll);
        AnimationManager::RegisterEvent("GTS_Sneak_Vore_SmileOn", "SneakVore", GTS_Sneak_Vore_SmileOn);
        AnimationManager::RegisterEvent("GTS_Sneak_Vore_SmileOff", "SneakVore", GTS_Sneak_Vore_SmileOff);
	}
}