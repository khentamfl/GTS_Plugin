// Animation: ThighCrush
//  - Stages
//    - "GTStosit",                     // [0] Start air rumble and camera shake
//    - "GTSsitloopenter",              // [1] Sit down completed
//    - "GTSsitloopstart",              // [2] enter sit crush loop
//    - "GTSsitloopend",                // [3] unused
//    - "GTSsitcrushlight_start",       // [4] Start Spreading legs
//    - "GTSsitcrushlight_end",         // [5] Legs fully spread
//    - "GTSsitcrushheavy_start",       // [6] Start Closing legs together
//    - "GTSsitcrushheavy_end",         // [7] Legs fully closed
//    - "GTSsitloopexit",               // [8] stand up, small air rumble and camera shake
//    - "GTSstandR",                    // [9] feet collides with ground when standing up
//    - "GTSstandL",                    // [10]
//    - "GTSstandRS",                   // [11] Silent impact of right feet
//    - "GTStoexit",                    // [12] Leave animation, disable air rumble and such

#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/animation/ThighCrush.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/InputManager.hpp"
#include "managers/CrushManager.hpp"
#include "managers/explosion.hpp"
#include "managers/footstep.hpp"
#include "managers/tremor.hpp"
#include "managers/Rumble.hpp"
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

	const std::vector<std::string_view> BODY_RUMBLE_NODES = { // used for body rumble
		"NPC COM [COM ]",
		"NPC L Foot [Lft ]",
		"NPC R Foot [Rft ]",
		"NPC L Toe0 [LToe]",
		"NPC R Toe0 [RToe]",
		"NPC L Calf [LClf]",
		"NPC R Calf [RClf]",
		"NPC L PreRearCalf",
		"NPC R PreRearCalf",
		"NPC L FrontThigh",
		"NPC R FrontThigh",
		"NPC R RearCalf [RrClf]",
		"NPC L RearCalf [RrClf]",
	};

	const std::vector<std::string_view> LEG_RUMBLE_NODES = { // used with Anim_ThighCrush
		"NPC L Foot [Lft ]",
		"NPC R Foot [Rft ]",
		"NPC L Toe0 [LToe]",
		"NPC R Toe0 [RToe]",
		"NPC L Calf [LClf]",
		"NPC R Calf [RClf]",
		"NPC L PreRearCalf",
		"NPC R PreRearCalf",
		"NPC L FrontThigh",
		"NPC R FrontThigh",
		"NPC R RearCalf [RrClf]",
		"NPC L RearCalf [RrClf]",
	};

	void LegRumbleOnce(std::string_view tag, Actor& actor, float power, float halflife) {
		for (auto& node_name: LEG_RUMBLE_NODES) {
			std::string rumbleName = std::format("{}{}", tag, node_name);
			Rumble::Once(rumbleName, &actor, power,  halflife, node_name);
		}
	}

	void StartLegRumble(std::string_view tag, Actor& actor, float power, float halflife) {
		for (auto& node_name: LEG_RUMBLE_NODES) {
			std::string rumbleName = std::format("{}{}", tag, node_name);
			Rumble::Start(rumbleName, &actor, power,  halflife, node_name);
		}
	}

	void StartBodyRumble(std::string_view tag, Actor& actor, float power, float halflife) {
		for (auto& node_name: BODY_RUMBLE_NODES) {
			std::string rumbleName = std::format("{}{}", tag, node_name);
			Rumble::Start(rumbleName, &actor, power,  halflife, node_name);
		}
	}

	void StopLegRumble(std::string_view tag, Actor& actor) {
		for (auto& node_name: LEG_RUMBLE_NODES) {
			std::string rumbleName = std::format("{}{}", tag, node_name);
			Rumble::Stop(rumbleName, &actor);
		}
	}

	void GTStosit(AnimationEventData& data) {
		float scale = get_visual_scale(data.giant);
		float speed = data.animSpeed;
		StartLegRumble("ThighCrush", data.giant, 0.10, 0.10);
		TrackFeet(&data.giant, 0.0, true); // Track feet
		data.stage = 1;
	}

	void GTSsitloopenter(AnimationEventData& data) {
		float scale = get_visual_scale(data.giant);
		float speed = data.animSpeed;
		StartLegRumble("ThighCrush", data.giant, 0.12 * speed, 0.10);
		data.disableHH = true;
		data.stage = 2;
	}

	void GTSsitloopstart(AnimationEventData& data) {
		float scale = get_visual_scale(data.giant);
		float speed = data.animSpeed;
		StopLegRumble("ThighCrush", data.giant);
		data.currentTrigger = 1;
		data.stage = 3;
	}

	void GTSsitloopend(AnimationEventData& data) {
		data.stage = 4;
	}

	void GTSsitcrushlight_start(AnimationEventData& data) {
		StartLegRumble("ThighCrush", data.giant, 0.18, 0.12);
		DrainStamina(&data.giant, "StaminaDrain_Thighs", "KillerThighs", true, 0.25, 1.0); // < Start Light Stamina Drain
		data.stage = 5;
	}

	void GTSsitcrushlight_end(AnimationEventData& data) {
		data.currentTrigger = 2;
		data.canEditAnimSpeed = true;
		LegRumbleOnce("ThighCrush_End", data.giant, 0.22, 0.20);
		StopLegRumble("ThighCrush", data.giant);
		DrainStamina(&data.giant, "StaminaDrain_Thighs", "KillerThighs", false, 0.25, 1.0); // < Stop Light Stamina Drain
		data.stage = 6;
	}

	void GTSsitcrushheavy_start(AnimationEventData& data) {
		DrainStamina(&data.giant, "StaminaDrain_Thighs", "KillerThighs", true, 0.25, 2.5); // < - Start HEAVY Stamina Drain
		StartLegRumble("ThighCrushHeavy", data.giant, 0.35, 0.10);
		data.stage = 5;
	}

	void GTSsitcrushheavy_end(AnimationEventData& data) {
		data.currentTrigger = 2;
		DrainStamina(&data.giant, "StaminaDrain_Thighs", "KillerThighs", false, 0.25, 2.5); // < Stop Heavy Stamina Drain
		LegRumbleOnce("ThighCrushHeavy_End", data.giant, 0.50, 0.15);
		StopLegRumble("ThighCrushHeavy", data.giant);
		data.stage = 6;
	}

	void GTSsitloopexit(AnimationEventData& data) {
		float scale = get_visual_scale(data.giant);
		float speed = data.animSpeed;

		data.disableHH = false;
		data.canEditAnimSpeed = false;
		data.animSpeed = 1.0;

		StartBodyRumble("BodyRumble", data.giant, 0.25, 0.12);
		data.stage = 8;
	}

	void GTSstandR(AnimationEventData& data) {
		float scale = get_visual_scale(data.giant);
		float speed = data.animSpeed;
		float volume = scale * 0.10 * speed;
		float perk = GetPerkBonus_Thighs(&data.giant);

		Rumble::Once("ThighCrushStompR", &data.giant, volume * 4, 0.10, RNode);
		DoSizeEffect(&data.giant, 0.75, FootEvent::Right, RNode);
		DoDamageEffect(&data.giant, 0.6 * perk, 1.1, 25, 0.03, FootEvent::Right);
		data.stage = 9;
		//Cprint("ThighCrush: GTSstandR");
	}

	void GTSstandL(AnimationEventData& data) {
		float scale = get_visual_scale(data.giant);
		float speed = data.animSpeed;
		float volume = scale * 0.10 * speed;
		float perk = GetPerkBonus_Thighs(&data.giant);

		Rumble::Once("ThighCrushStompL", &data.giant, volume * 4, 0.10, LNode);
		DoSizeEffect(&data.giant, 0.75, FootEvent::Left, LNode);
		DoDamageEffect(&data.giant, 0.6 * perk, 1.1, 25, 0.03, FootEvent::Left);
		data.stage = 9;
		//Cprint("ThighCrush: GTSstandL");
	}

	void GTSstandRS(AnimationEventData& data) {
		float scale = get_visual_scale(data.giant);
		float speed = data.animSpeed;
		float volume = scale * 0.05 * speed;
		float perk = GetPerkBonus_Thighs(&data.giant);

		Rumble::Once("ThighCrushStompR", &data.giant, volume * 4, 0.10, RNode);
		DoSizeEffect(&data.giant, 0.50, FootEvent::Right, RNode);
		DoDamageEffect(&data.giant, 0.6 * perk, 0.9, 25, 0.03, FootEvent::Right);
		data.stage = 9;
	}
	void GTSBEH_Next(AnimationEventData& data) {
		// Inbetween stages
		data.animSpeed = 1.0;
		data.canEditAnimSpeed = false;
	}
	void GTStoexit(AnimationEventData& data) {
		// Going to exit
		StopLegRumble("BodyRumble", data.giant);
		TrackFeet(&data.giant, 0.0, false); // Un-track feet
	}
	void GTSBEH_Exit(AnimationEventData& data) {
		// Final exit
		data.stage = 0;
	}

	void ThighCrushEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		AnimationManager::StartAnim("ThighLoopEnter", player);
	}

	void ThighCrushKillEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		float WasteStamina = 40.0;
		if (Runtime::HasPerk(player, "KillerThighs")) {
			WasteStamina *= 0.65;
		}
		if (GetAV(player, ActorValue::kStamina) > WasteStamina) {
			AnimationManager::StartAnim("ThighLoopAttack", player);
		} else {
			if (IsGtsBusy(player)) {
				TiredSound(player, "You're too tired to perform thighs attack");
			}
		}
	}

	void ThighCrushSpareEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		AnimationManager::StartAnim("ThighLoopExit", player);
	}
}

namespace Gts
{
	void AnimationThighCrush::RegisterEvents() {
		AnimationManager::RegisterEvent("GTStosit", "ThighCrush", GTStosit);
		AnimationManager::RegisterEvent("GTSsitloopenter", "ThighCrush", GTSsitloopenter);
		AnimationManager::RegisterEvent("GTSsitloopstart", "ThighCrush", GTSsitloopstart);
		AnimationManager::RegisterEvent("GTSsitloopend", "ThighCrush", GTSsitloopend);
		AnimationManager::RegisterEvent("GTSsitcrushlight_start", "ThighCrush", GTSsitcrushlight_start);
		AnimationManager::RegisterEvent("GTSsitcrushlight_end", "ThighCrush", GTSsitcrushlight_end);
		AnimationManager::RegisterEvent("GTSsitcrushheavy_start", "ThighCrush", GTSsitcrushheavy_start);
		AnimationManager::RegisterEvent("GTSsitcrushheavy_end", "ThighCrush", GTSsitcrushheavy_end);
		AnimationManager::RegisterEvent("GTSsitloopexit", "ThighCrush", GTSsitloopexit);
		AnimationManager::RegisterEvent("GTSstandR", "ThighCrush", GTSstandR);
		AnimationManager::RegisterEvent("GTSstandL", "ThighCrush", GTSstandL);
		AnimationManager::RegisterEvent("GTSstandRS", "ThighCrush", GTSstandRS);
		AnimationManager::RegisterEvent("GTStoexit", "ThighCrush", GTStoexit);
		AnimationManager::RegisterEvent("GTSBEH_Next", "ThighCrush", GTSBEH_Next);
		AnimationManager::RegisterEvent("GTSBEH_Exit", "ThighCrush", GTSBEH_Exit);

		InputManager::RegisterInputEvent("ThighCrush", ThighCrushEvent);
		InputManager::RegisterInputEvent("ThighCrushKill", ThighCrushKillEvent);
		InputManager::RegisterInputEvent("ThighCrushSpare", ThighCrushSpareEvent);
	}

	void AnimationThighCrush::RegisterTriggers() {
		AnimationManager::RegisterTriggerWithStages("ThighCrush", "ThighCrush", {"GTSBeh_TriggerSitdown", "GTSBeh_StartThighCrush", "GTSBeh_LeaveSitdown"});
		AnimationManager::RegisterTrigger("ThighLoopEnter", "ThighCrush", "GTSBeh_TriggerSitdown");
		AnimationManager::RegisterTrigger("ThighLoopAttack", "ThighCrush", "GTSBeh_StartThighCrush");
		AnimationManager::RegisterTrigger("ThighLoopExit", "ThighCrush", "GTSBeh_LeaveSitdown");
		AnimationManager::RegisterTrigger("ThighLoopFull", "ThighCrush", "GTSBeh_ThighAnimationFull");
	}
}
