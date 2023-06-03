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

#include "managers/animation/Stomp.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/damage/AccurateDamage.hpp"
#include "managers/damage/LaunchActor.hpp"
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

	float GetPerkBonus(Actor* Giant) {
		if (Runtime::HasPerkTeam(Giant, "DestructionBasics")) {
			return 1.25;
		} else {
			return 1.0;
		}
	}

	void DrainStamina(Actor* giant, bool decide, float power) {
		float WasteMult = 1.0;
		if (Runtime::HasPerkTeam(giant, "DestructionBasics")) {
			WasteMult *= 0.65;
		}
		std::string name = std::format("StaminaDrain_Stomp_{}", giant->formID);
		if (decide) {
			TaskManager::Run(name, [=](auto& progressData) {
				ActorHandle casterhandle = giant->CreateRefHandle();
				if (!casterhandle) {
					return false;
				}
				float multiplier = AnimationManager::GetAnimSpeed(giant);
				float WasteStamina = 1.00 * power * multiplier;
				DamageAV(giant, ActorValue::kStamina, WasteStamina * WasteMult);
				return true;
			});
		} else {
			TaskManager::Cancel(name);
		}
	}

	void DoLaunch(Actor* giant, float radius, float damage, std::string_view node) {
		float bonus = 1.0;
		if (HasSMT(giant)) {
			bonus = 2.0; // Needed to boost only Launch
		}
		LaunchActor::GetSingleton().ApplyLaunch(giant, radius * bonus, damage, node);
	}

	void GTSstompstartR(AnimationEventData& data) {
		data.stage = 1;
		data.canEditAnimSpeed = true;
		data.animSpeed = 1.33;
		if (data.giant.formID != 0x14) {
			data.animSpeed = 1.33 + GetRandomBoost()/2;
		}
		DrainStamina(&data.giant, true, 1.0);
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
		DrainStamina(&data.giant, true, 1.0);
		TrackFeet(&data.giant, 5, true);
		Rumble::Start("StompL", &data.giant, 0.45, 0.15, LNode); // Start stonger effect
		log::info("StompStartL true");
	}

	void GTSstompimpactR(AnimationEventData& data) {
		float shake = 1.0;
		float launch = 1.0;
		float dust = 1.0;
		float perk = GetPerkBonus(&data.giant);
		if (Runtime::HasMagicEffect(&data.giant, "SmallMassiveThreat")) {
			shake = 4.0;
			launch = 1.5;
			dust = 1.25;
		}
		Rumble::Once("StompR", &data.giant, 2.20 * shake, 0.0, RNode);
		DoDamageEffect(&data.giant, 1.5 * launch * data.animSpeed * perk, 1.2 * launch * data.animSpeed, 10, 0.25);
		DoSizeEffect(&data.giant, 1.10 * data.animSpeed, FootEvent::Right, RNode, dust);
		DoLaunch(&data.giant, 1.0 * launch, 2.25, RNode);
		DrainStamina(&data.giant, false, 1.0);
	}

	void GTSstompimpactL(AnimationEventData& data) {
		float shake = 1.0;
		float launch = 1.0;
		float dust = 1.0;
		float perk = GetPerkBonus(&data.giant);
		if (Runtime::HasMagicEffect(&data.giant, "SmallMassiveThreat")) {
			shake = 4.0;
			launch = 1.5;
			dust = 1.25;
		}
		Rumble::Once("StompL", &data.giant, 2.20 * shake, 0.0, LNode);
		DoDamageEffect(&data.giant, 1.5 * launch * data.animSpeed * perk, 1.2 * launch * data.animSpeed, 10, 0.25);
		DoSizeEffect(&data.giant, 1.10 * data.animSpeed, FootEvent::Left, LNode, dust);
		DoLaunch(&data.giant, 1.0 * launch * perk, 2.25, LNode);
		DrainStamina(&data.giant, false, 1.0);
	}

	void GTSstomplandR(AnimationEventData& data) {
		//data.stage = 2;
		float bonus = 1.0;
		float perk = GetPerkBonus(&data.giant);
		if (Runtime::HasMagicEffect(&data.giant, "SmallMassiveThreat")) {
			bonus = 4.0;
		}
		Rumble::Start("StompRL", &data.giant, 0.45, 0.10, RNode);
		DoDamageEffect(&data.giant, 0.7 * perk, 1.10, 25, 0.25);
		DoSizeEffect(&data.giant, 0.85, FootEvent::Right, RNode, bonus);
		DoLaunch(&data.giant, 0.7 * bonus * perk, 1.2, RNode);
	}

	void GTSstomplandL(AnimationEventData& data) {
		//data.stage = 2;
		float bonus = 1.0;
		float perk = GetPerkBonus(&data.giant);
		if (Runtime::HasMagicEffect(&data.giant, "SmallMassiveThreat")) {
			bonus = 4.0;
		}
		Rumble::Start("StompLL", &data.giant, 0.45, 0.10, LNode);
		DoDamageEffect(&data.giant, 0.7 * perk, 1.10, 25, 0.25);
		DoSizeEffect(&data.giant, 0.85, FootEvent::Left, LNode, bonus);
		DoLaunch(&data.giant, 0.7 * bonus * perk, 1.2, LNode);

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
