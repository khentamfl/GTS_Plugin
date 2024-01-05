#include "managers/animation/Controllers/ButtCrushController.hpp"
#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/animation/Utils/CrawlUtils.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/damage/CollisionDamage.hpp"
#include "managers/animation/SneakCrush.hpp"
#include "managers/damage/LaunchActor.hpp"
#include "managers/ai/aifunctions.hpp"
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

// Butt Crush alternative for sneaking
// Has Knee and Butt crush

namespace {

    const std::string_view RNode = "NPC R Foot [Rft ]";
	const std::string_view LNode = "NPC L Foot [Lft ]";

    void TrackKnee(Actor* giant, bool enable) {
		if (AllowFeetTracking()) {
			auto& sizemanager = SizeManager::GetSingleton();
			sizemanager.SetActionBool(giant, enable, 8.0);
		}
	}

    void TrackBooty(Actor* giant, bool enable) {
		if (AllowFeetTracking()) {
			auto& sizemanager = SizeManager::GetSingleton();
			sizemanager.SetActionBool(giant, enable, 8.0);
		}
	}

    void DoFootsteps(Actor* giant, float power, bool right) {
        float dust = 1.0;
        float perk = GetPerkBonus_Basics(giant);

        if (HasSMT(giant)) {
			dust = 1.25;
		} 
        
        if (right) {
            GRumble::Once("FST_R", giant, 2.20 * power, 0.0, RNode);
            DoDamageEffect(giant, 1.4 * power, 1.45 * power, 10, 0.25, FootEvent::Right, 1.0, DamageSource::CrushedRight);
            DoFootstepSound(giant, 1.0 * power, FootEvent::Right, RNode);
            DoDustExplosion(giant, dust * power, FootEvent::Right, RNode);
            DoLaunch(giant, 0.65 * perk * power, 1.3 * power, FootEvent::Right);
        } else {
            GRumble::Once("FST_L", giant, 2.20 * power, 0.0, LNode);
            DoDamageEffect(giant, 1.4 * power, 1.45 * power, 10, 0.25, FootEvent::Left, 1.0, DamageSource::CrushedLeft);
            DoFootstepSound(giant, 1.0 * power, FootEvent::Left, LNode);
            DoDustExplosion(giant, dust * power, FootEvent::Left, LNode);
            DoLaunch(giant, 0.65 * perk * power, 1.3 * power, FootEvent::Left);
        }
    }


    void DoButtDamage(Actor* giant) {
		float perk = GetPerkBonus_Basics(giant);
		float dust = 1.0;

		if (HasSMT(giant)) {
			dust = 1.25;
		}

		SetBonusSize(giant, 0.0, true);

		float damage = GetButtCrushDamage(giant);
		auto ThighL = find_node(giant, "NPC L Thigh [LThg]");
		auto ThighR = find_node(giant, "NPC R Thigh [RThg]");
		auto ButtR = find_node(giant, "NPC R Butt");
		auto ButtL = find_node(giant, "NPC L Butt");
		if (ButtR && ButtL) {
			if (ThighL && ThighR) {
				DoDamageAtPoint(giant, 28, 280.0 * damage, ThighL, 4, 0.70, 0.85, DamageSource::Booty);
				DoDamageAtPoint(giant, 28, 280.0 * damage, ThighR, 4, 0.70, 0.85, DamageSource::Booty);
				DoDustExplosion(giant, 1.45 * dust * damage, FootEvent::Right, "NPC R Butt");
				DoDustExplosion(giant, 1.45 * dust * damage, FootEvent::Left, "NPC L Butt");
				DoFootstepSound(giant, 1.25, FootEvent::Right, RNode);
				DoLaunch(giant, 1.30 * perk, 4.20, FootEvent::Butt);
				GRumble::Once("Butt_L", giant, 3.60 * damage, 0.02, "NPC R Butt");
				GRumble::Once("Butt_R", giant, 3.60 * damage, 0.02, "NPC L Butt");
			}
		} else {
			if (!ButtR) {
				Notify("Error: Missing Butt Nodes"); // Will help people to troubleshoot it. Not everyone has 3BB/XPMS32 body.
				Notify("Error: effects not inflicted");
				Notify("install 3BBB/XP32 Skeleton");
			}
			if (!ThighL) {
				Notify("Error: Missing Thigh Nodes");
				Notify("Error: effects not inflicted");
				Notify("install 3BBB/XP32 Skeleton");
			}
		}
		ModGrowthCount(giant, 0, true); // Reset limit
    }

    void DoKneeDamage(Actor* giant) {

		float perk = GetPerkBonus_Basics(giant);
		float dust = 1.0;

		if (HasSMT(giant)) {
			dust = 1.25;
		}

		SetBonusSize(giant, 0.0, true);

		float damage = GetButtCrushDamage(giant);
		auto LeftKnee = find_node(giant, "NPC L Calf [LClf]");
		auto RightKnee = find_node(giant, "NPC R Calf [RClf]");
		if (LeftKnee && RightKnee) {
            DoDamageAtPoint(giant, 28, 330.0 * damage, LeftKnee, 4, 0.70, 0.85, DamageSource::KneeLeft);
            DoDamageAtPoint(giant, 28, 330.0 * damage, RightKnee, 4, 0.70, 0.85, DamageSource::KneeRight);

            DoDustExplosion(giant, 1.45 * dust * damage, FootEvent::Left, "NPC L Calf [LClf]");
            DoDustExplosion(giant, 1.45 * dust * damage, FootEvent::Right, "NPC R Calf [RClf]");
            
            DoFootstepSound(giant, 1.25, FootEvent::Left, "NPC L Calf [LClf]");
            DoFootstepSound(giant, 1.25, FootEvent::Right, "NPC R Calf [RClf]");

            LaunchActor::GetSingleton().LaunchAtNode(giant, 1.30 * perk, 4.20, "NPC L Calf [LClf]");
            LaunchActor::GetSingleton().LaunchAtNode(giant, 1.30 * perk, 4.20, "NPC R Calf [RClf]");

            GRumble::Once("Knee_L", giant, 3.60 * damage, 0.02, "NPC L Calf [LClf]");
            GRumble::Once("Knee_R", giant, 3.60 * damage, 0.02, "NPC R Calf [RClf]");
		} else {
			if (!LeftKnee) {
				Notify("Error: Missing Knee Nodes"); // Will help people to troubleshoot it. Not everyone has 3BB/XP32 body.
				Notify("Error: effects not inflicted");
				Notify("install 3BBB/XP32 Skeleton");
			}
		}
		ModGrowthCount(giant, 0, true); // Reset limit
    }


    /////////////////////////////////////////////////////////////////////////////////////////
    ///                             E V E N T S
    ////////////////////////////////////////////////////////////////////////////////////////

    void GTSSneakCrush_Knee_CamOn(AnimationEventData& data) {TrackKnee(&data.giant, true);}
    void GTSSneakCrush_Knee_CamOff(AnimationEventData& data) {TrackKnee(&data.giant, false);}
    // Knee / Butt camera tracking
    void GTSSneakCrush_Butt_CamOn(AnimationEventData& data) {TrackBooty(&data.giant, true);}
    void GTSSneakCrush_Butt_CamOff(AnimationEventData& data) {TrackBooty(&data.giant, false);}
    // Knee / Butt impacts
    void GTSSneakCrush_Knee_FallDownImpact(AnimationEventData& data) {DoKneeDamage(&data.giant);}
    void GTSSneakCrush_Butt_FallDownImpact(AnimationEventData& data) {DoButtDamage(&data.giant);}

    // footsteps 
    void GTSSneakCrush_FootStepL(AnimationEventData& data) {DoFootstepEffects(&data.giant, 1.0, false);}
    void GTSSneakCrush_FootStepR(AnimationEventData& data) {DoFootstepEffects(&data.giant, 1.0, true);}

    void GTSSneakCrush_FootStep_SilentL(AnimationEventData& data) {DoFootstepEffects(&data.giant, 0.7, false);}
    void GTSSneakCrush_FootStep_SilentR(AnimationEventData& data) {DoFootstepEffects(&data.giant, 0.7, true);}
}

namespace Gts
{
	void AnimationSneakCrush::RegisterEvents() {
		AnimationManager::RegisterEvent("GTSSneakCrush_Knee_CamOn", "SneakCrush", GTSSneakCrush_Knee_CamOn);
		AnimationManager::RegisterEvent("GTSSneakCrush_Knee_CamOff", "ButtCrush", GTSSneakCrush_Knee_CamOff);
		AnimationManager::RegisterEvent("GTSSneakCrush_Butt_CamOn", "ButtCrush", GTSSneakCrush_Butt_CamOn);
		AnimationManager::RegisterEvent("GTSSneakCrush_Butt_CamOff", "ButtCrush", GTSSneakCrush_Butt_CamOff);
		AnimationManager::RegisterEvent("GTSSneakCrush_Knee_FallDownImpact", "ButtCrush", GTSSneakCrush_Knee_FallDownImpact);
		AnimationManager::RegisterEvent("GTSSneakCrush_Butt_FallDownImpact", "ButtCrush", GTSSneakCrush_Butt_FallDownImpact);
		AnimationManager::RegisterEvent("GTSSneakCrush_FootStepL", "ButtCrush", GTSSneakCrush_FootStepL);
		AnimationManager::RegisterEvent("GTSSneakCrush_FootStepR", "ButtCrush", GTSSneakCrush_FootStepR);
	}
}