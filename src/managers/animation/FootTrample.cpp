#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/damage/AccurateDamage.hpp"
#include "managers/animation/FootTrample.hpp"
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

    void GTS_Trample_Leg_Raise_L(AnimationEventData& data) {
        TrackFeet(&data.giant, 5, true);
    }
    void GTS_Trample_Leg_Raise_R(AnimationEventData& data) {
        TrackFeet(&data.giant, 6, true);
    } 
    void GTS_Trample_Cam_End_L(AnimationEventData& data) {
        TrackFeet(&data.giant, 5, false);
    }
    void GTS_Trample_Cam_End_R(AnimationEventData& data) {
        TrackFeet(&data.giant, 6, false);
    }


    void GTS_Trample_Impact_L(AnimationEventData& data) { // First stomp that then determines if .dll should launch the rest of anim
        float shake = 1.0;
		float launch = 1.0;
		float dust = 1.0;
		float perk = GetPerkBonus_Basics(&data.giant);
		if (HasSMT(&data.giant)) {
			shake = 4.0;
			launch = 1.2;
			dust = 1.25;
		}
		GRumble::Once("TrampleL", &data.giant, 1.60 * shake, 0.0, LNode);
		DoDamageEffect(&data.giant, 1.2 * launch * perk, 1.45 * launch, 10, 0.25, FootEvent::Left, 1.0, DamageSource::CrushedLeft);
		DoFootstepSound(&data.giant, 1.0, FootEvent::Left, LNode);
		DoDustExplosion(&data.giant, dust, FootEvent::Left, LNode);
		DoLaunch(&data.giant, 0.75 * launch * perk, 2.25, 1.0, FootEvent::Left, 0.95);
		DrainStamina(&data.giant, "StaminaDrain_Trample", "DestructionBasics", false, 1.0, 1.8);
		FootGrindCheck_Left(&data.giant, 1.60, true);
    }

    void GTS_Trample_Impact_R(AnimationEventData& data) { // First stomp that then determines if .dll should launch the rest of anim
        float shake = 1.0;
		float launch = 1.0;
		float dust = 1.0;
		float perk = GetPerkBonus_Basics(&data.giant);
		if (HasSMT(&data.giant)) {
			shake = 4.0;
			launch = 1.2;
			dust = 1.25;
		}
		GRumble::Once("TrampleL", &data.giant, 1.60 * shake, 0.0, LNode);
		DoDamageEffect(&data.giant, 1.2 * launch * perk, 1.45 * launch, 10, 0.25, FootEvent::Right, 1.0, DamageSource::CrushedRight);
		DoFootstepSound(&data.giant, 1.0, FootEvent::Right, RNode);
		DoDustExplosion(&data.giant, dust, FootEvent::Right, RNode);
		DoLaunch(&data.giant, 0.75 * launch * perk, 2.25, 1.0, FootEvent::Right, 0.95);
		DrainStamina(&data.giant, "StaminaDrain_Trample", "DestructionBasics", false, 1.0, 1.8);
		FootGrindCheck_Left(&data.giant, 1.60, true);
    }

    void GTS_Trample_Footstep_L(AnimationEventData& data) { // Attack that happen after we 
        float shake = 1.0;
		float launch = 1.0;
		float dust = 1.0;
		float perk = GetPerkBonus_Basics(&data.giant);
		if (HasSMT(&data.giant)) {
			shake = 4.0;
			launch = 1.2;
			dust = 1.25;
		}
		GRumble::Once("TrampleL", &data.giant, 2.40 * shake, 0.0, LNode);
		DoDamageEffect(&data.giant, 1.8 * launch * perk, 1.75 * launch, 10, 0.25, FootEvent::Left, 1.0, DamageSource::CrushedLeft);
		DoFootstepSound(&data.giant, 1.0, FootEvent::Left, LNode);
		DoDustExplosion(&data.giant, dust, FootEvent::Left, LNode);
		DoLaunch(&data.giant, 0.75 * launch * perk, 2.25, 1.0, FootEvent::Left, 0.95);
		DrainStamina(&data.giant, "StaminaDrain_Trample", "DestructionBasics", false, 1.0, 1.8);
    }
    void GTS_Trample_Footstep_R(AnimationEventData& data) {
        float shake = 1.0;
		float launch = 1.0;
		float dust = 1.0;
		float perk = GetPerkBonus_Basics(&data.giant);
		if (HasSMT(&data.giant)) {
			shake = 4.0;
			launch = 1.2;
			dust = 1.25;
		}
		GRumble::Once("TrampleL", &data.giant, 2.40 * shake, 0.0, LNode);
		DoDamageEffect(&data.giant, 1.8 * launch * perk, 1.75 * launch, 10, 0.25, FootEvent::Right, 1.0, DamageSource::CrushedRight);
		DoFootstepSound(&data.giant, 1.0, FootEvent::Right, RNode);
		DoDustExplosion(&data.giant, dust, FootEvent::Right, RNode);
		DoLaunch(&data.giant, 0.75 * launch * perk, 2.25, 1.0, FootEvent::Right, 0.95);
		DrainStamina(&data.giant, "StaminaDrain_Trample", "DestructionBasics", false, 1.0, 1.8);
    }

    void GTS_Trample_Finisher_L(AnimationEventData& data) { // last hit that always kills the target
        
    }
    void GTS_Trample_Finisher_R(AnimationEventData& data) { // last hit that always kills the target
        
    }

    /////////////////////////////////////////////////////////// Triggers

    void TrampleLeftEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		if (!CanPerformAnimation(player, 1)) {
			return;
		}
		float WasteStamina = 35.0;
		if (Runtime::HasPerk(player, "DestructionBasics")) {
			WasteStamina *= 0.65;
		}
		if (GetAV(player, ActorValue::kStamina) > WasteStamina) {
			AnimationManager::StartAnim("TrampleL", player);
		} else {
			TiredSound(player, "You're too tired to perform trample");
		}
	}

    void TrampleRightEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		if (!CanPerformAnimation(player, 1)) {
			return;
		}
		float WasteStamina = 35.0;
		if (Runtime::HasPerk(player, "DestructionBasics")) {
			WasteStamina *= 0.65;
		}
		if (GetAV(player, ActorValue::kStamina) > WasteStamina) {
			AnimationManager::StartAnim("TrampleR", player);
		} else {
			TiredSound(player, "You're too tired to perform trample");
		}
	}
}

namespace Gts
{
	void AnimationFootTrample::RegisterEvents() {
        InputManager::RegisterInputEvent("TrampleLeft", TrampleLeftEvent);
        InputManager::RegisterInputEvent("TrampleRight", TrampleRightEvent);

        AnimationManager::RegisterEvent("GTS_Trample_Leg_Raise_L", "Trample", GTS_Trample_Leg_Raise_L);
        AnimationManager::RegisterEvent("GTS_Trample_Leg_Raise_R", "Trample", GTS_Trample_Leg_Raise_R);

        AnimationManager::RegisterEvent("GTS_Trample_Cam_End_L", "Trample", GTS_Trample_Cam_End_L);
        AnimationManager::RegisterEvent("GTS_Trample_Cam_End_R", "Trample", GTS_Trample_Cam_End_R);
        
        AnimationManager::RegisterEvent("GTS_Trample_Impact_L", "Trample", GTS_Trample_Impact_L);
        AnimationManager::RegisterEvent("GTS_Trample_Impact_R", "Trample", GTS_Trample_Impact_R);

        AnimationManager::RegisterEvent("GTS_Trample_Footstep_L", "Trample", GTS_Trample_Footstep_L);
        AnimationManager::RegisterEvent("GTS_Trample_Footstep_R", "Trample", GTS_Trample_Footstep_R);

        AnimationManager::RegisterEvent("GTS_Trample_Finisher_L", "Trample", GTS_Trample_Finisher_L);
        AnimationManager::RegisterEvent("GTS_Trample_Finisher_R", "Trample", GTS_Trample_Finisher_R);
 
	}

	void AnimationFootTrample::RegisterTriggers() {
		AnimationManager::RegisterTrigger("TrampleL", "Trample", "GTSBeh_Trample_L");
        AnimationManager::RegisterTrigger("TrampleR", "Trample", "GTSBeh_Trample_R");

        AnimationManager::RegisterTrigger("TrampleStartL", "Trample", "GTSBEH_Trample_Start_L");
        AnimationManager::RegisterTrigger("TrampleStartR", "Trample", "GTSBEH_Trample_Start_R");
	}
}