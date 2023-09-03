#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/emotions/EmotionManager.hpp"
#include "managers/damage/AccurateDamage.hpp"
#include "managers/damage/LaunchActor.hpp"
#include "managers/animation/Kicks.hpp"
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
    const std::string_view RNode = "NPC R Foot [Rft ]";
	const std::string_view LNode = "NPC L Foot [Lft ]";

    void StartDamageAt(Actor* actor, float power, float crush, float pushpower, std::string_view node) {
		std::string name = std::format("LegKick_{}", actor->formID);
		auto gianthandle = actor->CreateRefHandle();
		TaskManager::Run(name, [=](auto& progressData) {
			if (!gianthandle) {
				return false;
			}
			auto giant = gianthandle.get().get();
			auto Leg = find_node(giant, node);
			if (Leg) {
				DoDamageAtPoint_Cooldown(giant, 17, 80.0 * power, Uarm, 10, 0.30, crush, pushpower);
			}
			return true;
		});
	}

    void StopAllDamageAndStamina(Actor* actor) {
        std::string name = std::format("LegKick_{}", actor->formID);
        DrainStamina(actor, "StaminaDrain_StrongKick", "DestructionBasics", false, 2.0, 8.0);
        DrainStamina(actor, "StaminaDrain_Kick", "DestructionBasics", false, 2.0, 4.0);
        TaskManager::CancelTask(name);
    }

    void GTS_Kick_SwingLeg_L(AnimationEventData& data) {}
    void GTS_Kick_SwingLeg_R(AnimationEventData& data) {}

    void GTS_Kick_Stomp_R(AnimationEventData& data) {
        DoDamageEffect(&data.giant, 1.40, 1.6, 10, 0.20, FootEvent::Right, 1.0, DamageSource::Crushed);
		DoFootstepSound(&data.giant, 1.0, FootEvent::Right, RNode);
		DoDustExplosion(&data.giant, 1.0, FootEvent::Right, RNode);
		DoLaunch(&data.giant, 0.85, 1.75, 1.4, FootEvent::Right, 0.85);
		
    }
    void GTS_Kick_Stomp_L(AnimationEventData& data) {
        DoDamageEffect(&data.giant, 1.40, 1.6, 10, 0.20, FootEvent::Left, 1.0, DamageSource::Crushed);
        DoFootstepSound(&data.giant, 1.0, FootEvent::Left, LNode);
        DoDustExplosion(&data.giant, 1.0, FootEvent::Left, LNode);
        DoLaunch(&data.giant, 0.85, 1.75, 1.4, FootEvent::Left, 0.85);
    }

    void GTS_Kick_HitBox_On_R(AnimationEventData& data) {
        StartDamageAt(&data.giant, 1.4, 1.8, 2.0, "NPC R Toe0 [RToe]");
		DrainStamina(&data.giant, "StaminaDrain_StrongKick", "DestructionBasics", true, 2.0, 4.0);
    }
    void GTS_Kick_HitBox_On_L(AnimationEventData& data) {
        StartDamageAt(&data.giant, 1.4, 1.8, 2.0, "NPC L Toe0 [LToe]");
		DrainStamina(&data.giant, "StaminaDrain_StrongKick", "DestructionBasics", true, 2.0, 4.0);
    }
    void GTS_Kick_HitBox_Off_R(AnimationEventData& data) {
        StopAllDamageAndStamina(&data.giant);
    }
    void GTS_Kick_HitBox_Off_L(AnimationEventData& data) {
        StopAllDamageAndStamina(&data.giant);
    }

    void GTS_Kick_HitBox_Power_On_R(AnimationEventData& data) {
        StartDamageAt(&data.giant, 3.0, 1.8, 4.0, "NPC R Toe0 [RToe]");
		DrainStamina(&data.giant, "StaminaDrain_StrongKick", "DestructionBasics", true, 2.0, 8.0);
    }
    void GTS_Kick_HitBox_Power_On_L(AnimationEventData& data) {
        StartDamageAt(&data.giant, 3.0, 1.8, 4.0, "NPC L Toe0 [LToe]");
		DrainStamina(&data.giant, "StaminaDrain_StrongKick", "DestructionBasics", true, 2.0, 8.0);
    }
    void GTS_Kick_HitBox_Power_Off_R(AnimationEventData& data) {
        StopAllDamageAndStamina(&data.giant);
    }
    void GTS_Kick_HitBox_Power_Off_L(AnimationEventData& data) {
        StopAllDamageAndStamina(&data.giant);
    }
}

namespace Gts
{
	void AnimationKicks::RegisterEvents() {
        AnimationManager::RegisterEvent("GTS_Kick_SwingLeg_R", "Kicks", GTS_Kick_SwingLeg_L);
        AnimationManager::RegisterEvent("GTS_Kick_SwingLeg_L", "Kicks", GTS_Kick_SwingLeg_R);

        AnimationManager::RegisterEvent("GTS_Kick_Stomp_R", "Kicks", GTS_Kick_Stomp_R);
        AnimationManager::RegisterEvent("GTS_Kick_Stomp_L", "Kicks", GTS_Kick_Stomp_L);
        
        AnimationManager::RegisterEvent("GTS_Kick_HitBox_On_R", "Kicks", GTS_Kick_HitBox_On_R);
        AnimationManager::RegisterEvent("GTS_Kick_HitBox_Off_R", "Kicks", GTS_Kick_HitBox_Off_R);
        AnimationManager::RegisterEvent("GTS_Kick_HitBox_On_L", "Kicks", GTS_Kick_HitBox_On_L);
        AnimationManager::RegisterEvent("GTS_Kick_HitBox_Off_L", "Kicks", GTS_Kick_HitBox_Off_L);

        AnimationManager::RegisterEvent("GTS_Kick_HitBox_Power_On_R", "Kicks", GTS_Kick_HitBox_Power_On_R);
        AnimationManager::RegisterEvent("GTS_Kick_HitBox_Power_Off_R", "Kicks", GTS_Kick_HitBox_Power_Off_R);
        AnimationManager::RegisterEvent("GTS_Kick_HitBox_Power_On_L", "Kicks", GTS_Kick_HitBox_Power_On_L);
        AnimationManager::RegisterEvent("GTS_Kick_HitBox_Power_Off_L", "Kicks", GTS_Kick_HitBox_Power_Off_L);
	}

	void AnimationKicks::RegisterTriggers() {
		AnimationManager::RegisterTrigger("KickStrongLeft", "Kicks", "GTSBeh_KickStrong_StartL");
        AnimationManager::RegisterTrigger("KickStrongRight", "Kicks", "GTSBeh_KickStrong_StartR");
        AnimationManager::RegisterTrigger("KickLeft", "Kicks", "GTSBeh_Kick_StartL");
        AnimationManager::RegisterTrigger("KickRight", "Kicks", "GTSBeh_Kick_StartR");
	}
}