// Animation: Stomp
//  - Stages
//    - "GTSstompimpactR",          // [0] stomp impacts, strongest effect
//    - "GTSstompimpactL",          // [1]
//    - "GTSstomplandR",            // [2] when landing after stomping, decreased power
//    - "GTSstomplandL",            // [3]
//    - "GTSstompstartR",           // [4] For starting loop of camera shake and air rumble sounds
//    - "GTSstompstartL",           // [5]
//    - "GTSStompendR",             // [6] disable loop of camera shake and air rumble sounds
//    - "GTSStompendL",             // [7]
//    - "GTS_Next",                 // [8]
//    - "GTSBEH_Exit",              // [9] Another disable

#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/damage/AccurateDamage.hpp"
#include "managers/damage/LaunchActor.hpp"
#include "managers/animation/Stomp.hpp"
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
	const std::string_view RNode = "NPC R Foot [Rft ]";
	const std::string_view LNode = "NPC L Foot [Lft ]";

	void GTSstompstartR(AnimationEventData& data) {
		data.stage = 1;
		data.canEditAnimSpeed = true;
		data.animSpeed = 1.33;
		if (data.giant.formID != 0x14) {
			data.animSpeed = 1.33 + GetRandomBoost()/2;
		}
		DrainStamina(&data.giant, "StaminaDrain_Stomp", "DestructionBasics", true, 1.0, 1.8);
		TrackFeet(&data.giant, 6, true);
		Rumble::Start("StompR", &data.giant, 0.35, 0.15, RNode);
		log::info("StompStartR true");
	}

	void GTSstompstartL(AnimationEventData& data) {
		data.stage = 1;
		data.canEditAnimSpeed = true;
		data.animSpeed = 1.33;
		if (data.giant.formID != 0x14) {
			data.animSpeed = 1.33 + GetRandomBoost()/2;
		}
		DrainStamina(&data.giant, "StaminaDrain_Stomp", "DestructionBasics", true, 1.0, 1.8);
		TrackFeet(&data.giant, 5, true);
		Rumble::Start("StompL", &data.giant, 0.45, 0.15, LNode); // Start stonger effect
		log::info("StompStartL true");
	}

	void GTSstompimpactR(AnimationEventData& data) {
		float shake = 1.0;
		float launch = 1.0;
		float dust = 1.25;
		float perk = GetPerkBonus_Basics(&data.giant);
		if (HasSMT(&data.giant)) {
			shake = 4.0;
			launch = 1.5;
			dust = 1.45;
		}
		Rumble::Once("StompR", &data.giant, 2.20 * shake, 0.0, RNode);
		DoDamageEffect(&data.giant, (2.45 + data.animSpeed/4) * launch * perk, (1.45 + data.animSpeed/4) * launch, 10, 0.03, FootEvent::Right);
		DoSizeEffect(&data.giant, dust + data.animSpeed/4, FootEvent::Right, RNode);
		DoLaunch(&data.giant, 0.75 * launch * perk, 2.25 * data.animSpeed, 2.0, FootEvent::Right, 0.75);
		DrainStamina(&data.giant, "StaminaDrain_Stomp", "DestructionBasics", false, 1.0, 1.8);
	}

	void GTSstompimpactL(AnimationEventData& data) {
		float shake = 1.0;
		float launch = 1.0;
		float dust = 1.25;
		float perk = GetPerkBonus_Basics(&data.giant);
		if (HasSMT(&data.giant)) {
			shake = 4.0;
			launch = 1.5;
			dust = 1.45;
		}
		Rumble::Once("StompL", &data.giant, 2.20 * shake, 0.0, LNode);
		DoDamageEffect(&data.giant, (2.45 + data.animSpeed/4) * launch * perk, (1.45 + data.animSpeed/4) * launch, 10, 0.03, FootEvent::Left);
		DoSizeEffect(&data.giant, dust + data.animSpeed/4, FootEvent::Left, LNode);
		DoLaunch(&data.giant, 0.75 * launch * perk, 2.25 * data.animSpeed, 2.0, FootEvent::Left, 0.75);
		DrainStamina(&data.giant, "StaminaDrain_Stomp", "DestructionBasics", false, 1.0, 1.8);
	}

	void GTSstomplandR(AnimationEventData& data) {
		//data.stage = 2;
		float shake = 1.0;
		float bonus = 1.0;
		float dust = 0.85;
		float perk = GetPerkBonus_Basics(&data.giant);
		if (HasSMT(&data.giant)) {
			bonus = 2.0;
			dust = 1.25;
			shake = 4.0;
		}
		Rumble::Once("StompRL", &data.giant, 1.25 * shake, 0.05, RNode);
		DoDamageEffect(&data.giant, 1.65 * perk, 1.45, 25, 0.025, FootEvent::Right);
		DoSizeEffect(&data.giant, dust + data.animSpeed/4, FootEvent::Right, RNode);
		DoLaunch(&data.giant, 0.75 * bonus * perk, 1.8 + data.animSpeed/4, 2.0, FootEvent::Right, 0.7);
	}

	void GTSstomplandL(AnimationEventData& data) {
		//data.stage = 2;
		float shake = 1.0;
		float bonus = 1.0;
		float dust = 0.85;
		float perk = GetPerkBonus_Basics(&data.giant);
		if (HasSMT(&data.giant)) {
			bonus = 2.0;
			dust = 1.25;
			shake = 4.0;
		}
		Rumble::Once("StompLL", &data.giant, 1.25 * shake, 0.05, LNode);
		DoDamageEffect(&data.giant, 1.65 * perk, 1.45, 25, 0.025, FootEvent::Left);
		DoSizeEffect(&data.giant, dust + data.animSpeed/4, FootEvent::Left, LNode);
		DoLaunch(&data.giant, 0.75 * bonus * perk, 1.8 + data.animSpeed/4, 2.0, FootEvent::Left, 0.7);

	}

	void GTSStompendR(AnimationEventData& data) {
		data.stage = 0;
		data.canEditAnimSpeed = false;
		data.animSpeed = 1.0;
	}

	void GTSStompendL(AnimationEventData& data) {
		data.stage = 0;
		data.canEditAnimSpeed = false;
		data.animSpeed = 1.0;
	}

	void GTS_Next(AnimationEventData& data) {
		Rumble::Stop("StompR", &data.giant);
		Rumble::Stop("StompL", &data.giant);
		Rumble::Stop("StompRL", &data.giant);
		Rumble::Stop("StompLL", &data.giant);
	}

	void GTSBEH_Exit(AnimationEventData& data) {
		Rumble::Stop("StompR", &data.giant);
		Rumble::Stop("StompL", &data.giant);
		DrainStamina(&data.giant, "StaminaDrain_Stomp", "DestructionBasics", false, 1.0, 1.8);
		DrainStamina(&data.giant, "StaminaDrain_StrongStomp", "DestructionBasics", false, 1.45, 2.8);
		TrackFeet(&data.giant, 6, false);
		TrackFeet(&data.giant, 5, false);
	}

	void RightStompEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		float WasteStamina = 25.0;
		if (Runtime::HasPerk(player, "DestructionBasics")) {
			WasteStamina *= 0.65;
		}
		if (GetAV(player, ActorValue::kStamina) > WasteStamina) {
			AnimationManager::StartAnim("StompRight", player);
		} else {
			TiredSound(player, "You're too tired to perform stomp");
		}
	}

	void LeftStompEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		float WasteStamina = 25.0;
		if (Runtime::HasPerk(player, "DestructionBasics")) {
			WasteStamina *= 0.65;
		}
		if (GetAV(player, ActorValue::kStamina) > WasteStamina) {
			AnimationManager::StartAnim("StompLeft", player);
		} else {
			TiredSound(player, "You're too tired to perform stomp");
		}
	}
}

namespace Gts
{
	void AnimationStomp::RegisterEvents() {
		AnimationManager::RegisterEvent("GTSstompimpactR", "Stomp", GTSstompimpactR);
		AnimationManager::RegisterEvent("GTSstompimpactL", "Stomp", GTSstompimpactL);
		AnimationManager::RegisterEvent("GTSstomplandR", "Stomp", GTSstomplandR);
		AnimationManager::RegisterEvent("GTSstomplandL", "Stomp", GTSstomplandL);
		AnimationManager::RegisterEvent("GTSstompstartR", "Stomp", GTSstompstartR);
		AnimationManager::RegisterEvent("GTSstompstartL", "Stomp", GTSstompstartL);
		AnimationManager::RegisterEvent("GTSStompendR", "Stomp", GTSStompendR);
		AnimationManager::RegisterEvent("GTSStompendL", "Stomp", GTSStompendL);
		AnimationManager::RegisterEvent("GTS_Next", "Stomp", GTS_Next);
		AnimationManager::RegisterEvent("GTSBEH_Exit", "Stomp", GTSBEH_Exit);

		InputManager::RegisterInputEvent("RightStomp", RightStompEvent);
		InputManager::RegisterInputEvent("LeftStomp", LeftStompEvent);
	}

	void AnimationStomp::RegisterTriggers() {
		AnimationManager::RegisterTrigger("StompRight", "Stomp", "GtsModStompAnimRight");
		AnimationManager::RegisterTrigger("StompLeft", "Stomp", "GtsModStompAnimLeft");
	}
}