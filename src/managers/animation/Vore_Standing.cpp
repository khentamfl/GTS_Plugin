#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/emotions/EmotionManager.hpp"
#include "managers/animation/Vore_Standing.hpp"
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


//                Sorted by order and timings
//GTSvore_sit_start                         //Start sit down and shake
//GTSvore_impactLS                          //Silent left feet impact
//GTSvore_sit_end                           //Sit end, stop shake
//GTSvore_hand_extend                       //Hand starts to move in space
//GTSvore_hand_grab                         //Hand grabs someoone
//GTSvore_attachactor_AnimObject_A          //Same as above
//GTSvore_bringactor_start                  //Hand starts to move victim in space
//GTSvore_open_mouth                        //Open mouth
//GTSvore_bringactor_end                    //Drop actor into mouth
//GTSvore_swallow                           //Swallow actor
//GTSvore_swallow_sound                     //Pretty much the same
//GTSvore_close_mouth                       //Close mouth
//GTSvore_handR_reposition_S                //Right hand starts to return to normal position
//GTSvore_handL_reposition_S                //Same but for left hand
//GTSvore_handR_reposition_E                //Right hand returned to normal position
//GTSvore_handL_reposition_E                //Same but for left hand
//GTSvore_eat_actor                         //Kill and eat actor completely
//GTSvore_detachactor_AnimObject_A          //Actor is no longer attached to AnimObjectA
//GTSvore_standup_start                     //Return to normal stance
//GTSvore_impactRS                          //Right feet collides with the ground
//GTSvore_standup_end                       //Exit animation

namespace {
	const std::string_view RNode = "NPC R Foot [Rft ]";
	const std::string_view LNode = "NPC L Foot [Lft ]";
	const std::string_view RSound = "lFootstepR";
	const std::string_view LSound = "lFootstepL";

	const std::vector<std::string_view> RHAND_RUMBLE_NODES = { // used for hand rumble
		"NPC R UpperarmTwist1 [RUt1]",
		"NPC R UpperarmTwist2 [RUt2]",
		"NPC R Forearm [RLar]",
		"NPC R ForearmTwist2 [RLt2]",
		"NPC R ForearmTwist1 [RLt1]",
		"NPC R Hand [RHnd]",
	};

	const std::vector<std::string_view> LHAND_RUMBLE_NODES = { // used for hand rumble
		"NPC L UpperarmTwist1 [LUt1]",
		"NPC L UpperarmTwist2 [LUt2]",
		"NPC L Forearm [LLar]",
		"NPC L ForearmTwist2 [LLt2]",
		"NPC L ForearmTwist1 [LLt1]",
		"NPC L Hand [LHnd]",
	};

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



	void StartBodyRumble(std::string_view tag, Actor& actor, float power, float halflife, bool once) {
		for (auto& node_name: BODY_RUMBLE_NODES) {
			std::string rumbleName = std::format("{}{}", tag, node_name);
			if (!once) {
				GRumble::Start(rumbleName, &actor, power,  halflife, node_name);
			} else {
				GRumble::Once(rumbleName, &actor, power, halflife, node_name);
			}
		}
	}

	void StartRHandRumble(std::string_view tag, Actor& actor, float power, float halflife) {
		for (auto& node_name: RHAND_RUMBLE_NODES) {
			std::string rumbleName = std::format("{}{}", tag, node_name);
			GRumble::Start(rumbleName, &actor, power,  halflife, node_name);
		}
	}

	void StartLHandRumble(std::string_view tag, Actor& actor, float power, float halflife) {
		for (auto& node_name: LHAND_RUMBLE_NODES) {
			std::string rumbleName = std::format("{}{}", tag, node_name);
			GRumble::Start(rumbleName, &actor, power,  halflife, node_name);
		}
	}

	void StopRHandRumble(std::string_view tag, Actor& actor) {
		for (auto& node_name: RHAND_RUMBLE_NODES) {
			std::string rumbleName = std::format("{}{}", tag, node_name);
			GRumble::Stop(rumbleName, &actor);
		}
	}
	void StopLHandRumble(std::string_view tag, Actor& actor) {
		for (auto& node_name: RHAND_RUMBLE_NODES) {
			std::string rumbleName = std::format("{}{}", tag, node_name);
			GRumble::Stop(rumbleName, &actor);
		}
	}

	void GTSvore_sit_start(AnimationEventData& data) {
		auto giant = &data.giant;
		auto& VoreData = Vore::GetSingleton().GetVoreData(&data.giant);
		for (auto& tiny: VoreData.GetVories()) {
			AllowToBeCrushed(tiny, false);
			DisableCollisions(tiny, giant);
			SetBeingHeld(tiny, true);
		}
		if (Runtime::GetBool("FreeLookOnVore") && giant->formID == 0x14) {
			EnableFreeCamera();
		}
		if (!Runtime::GetBool("FreeLookOnVore") && giant->formID == 0x14) {
			ManageCamera(giant, true, 4.0);
		}
		StartBodyRumble("BodyRumble", data.giant, 0.35, 0.10, false);

	}

	void GTSvore_impactLS(AnimationEventData& data) {
		auto& VoreData = Vore::GetSingleton().GetVoreData(&data.giant);
		float scale = get_visual_scale(&data.giant);
		float volume = scale * 0.20 * (data.animSpeed * data.animSpeed);
		//AllowToDoVore(&data.giant, false); // Disallow repeating Vore for NPC's
		for (auto& tiny: VoreData.GetVories()) {
			tiny->NotifyAnimationGraph("GTS_EnterFear");
		}
		VoreData.AllowToBeVored(false);
		GRumble::Once("StompLS", &data.giant, 0.45, 0.10, LNode);
		DoFootstepSound(&data.giant, 0.90, FootEvent::Left, LNode);
		DoDustExplosion(&data.giant, 0.90, FootEvent::Left, LNode);
		DoDamageEffect(&data.giant, 0.5, 1.0, 30, 0.25, FootEvent::Left, 1.0, DamageSource::CrushedLeft);
	}

	void GTSvore_sit_end(AnimationEventData& data) {
		GRumble::Stop("BodyRumble", &data.giant);
		AdjustFacialExpression(&data.giant, 2, 1.0, "expression"); // smile (expression)
	}

	void GTSvore_hand_extend(AnimationEventData& data) {
		StartRHandRumble("HandR", data.giant, 0.75, 0.15);
		auto& VoreData = Vore::GetSingleton().GetVoreData(&data.giant);
		for (auto& tiny: VoreData.GetVories()) {
			tiny->NotifyAnimationGraph("GTS_ExitFear");
		}
	}

	void GTSvore_hand_grab(AnimationEventData& data) {
		auto giant = &data.giant;
		auto& VoreData = Vore::GetSingleton().GetVoreData(giant);
		VoreData.GrabAll();
		ToggleEmotionEdit(giant, true);
		auto firstTiny = VoreData.GetVories()[0];
		for (auto& tiny: VoreData.GetVories()) {
			tiny->NotifyAnimationGraph("JumpFall");
			Attacked(tiny, giant);
		}
		if (!Runtime::GetBool("FreeLookOnVore") && giant->formID == 0x14) {
			ManageCamera(giant, false, 4.0);
			ManageCamera(giant, true, 2.0);
		}

		StopRHandRumble("HandR", data.giant);
	}

	void GTSvore_attachactor_AnimObject_A(AnimationEventData& data) {
	}

	void GTSvore_bringactor_start(AnimationEventData& data) {
		AdjustFacialExpression(&data.giant, 3, 0.8, "phenome"); // Smile a bit (Mouth)
		StartRHandRumble("HandR", data.giant, 0.6, 0.175);
	}

	void GTSvore_open_mouth(AnimationEventData& data) {
		auto giant = &data.giant;
		auto& VoreData = Vore::GetSingleton().GetVoreData(giant);
		AdjustFacialExpression(giant, 0, 1.0, "phenome"); // Start opening mouth
		AdjustFacialExpression(giant, 1, 0.5, "phenome"); // Open it wider
		AdjustFacialExpression(giant, 0, 0.80, "modifier"); // blink L
		AdjustFacialExpression(giant, 1, 0.80, "modifier"); // blink R
	}

	void GTSvore_bringactor_end(AnimationEventData& data) {
		auto giant = &data.giant;
		auto& VoreData = Vore::GetSingleton().GetVoreData(&data.giant);
		StopRHandRumble("HandR", data.giant);
		VoreData.AllowToBeVored(true);
		for (auto& tiny: VoreData.GetVories()) {
			AllowToBeCrushed(tiny, true);
		}
	}


	void GTSvore_swallow(AnimationEventData& data) {
		auto giant = &data.giant;
		auto& VoreData = Vore::GetSingleton().GetVoreData(&data.giant);
		VoreData.EnableMouthShrinkZone(true);
		if (AllowDevourment()) {
			for (auto& tiny: VoreData.GetVories()) {
				CallDevourment(giant, tiny);
			}
		}
	}

	void GTSvore_swallow_sound(AnimationEventData& data) {
		AdjustFacialExpression(&data.giant, 3, 0.0, "phenome"); // Remove smile (Mouth)
	}

	void GTSvore_close_mouth(AnimationEventData& data) {
		auto giant = &data.giant;
		auto& VoreData = Vore::GetSingleton().GetVoreData(giant);

		VoreData.EnableMouthShrinkZone(false);
		AdjustFacialExpression(giant, 0, 0.0, "phenome"); // Close mouth
		AdjustFacialExpression(giant, 1, 0.0, "phenome"); // Close mouth
		for (auto& tiny: VoreData.GetVories()) {
			if (tiny->formID == 0x14) {
				PlayerCamera::GetSingleton()->cameraTarget = giant->CreateRefHandle();
			}
		}
	}

	void GTSvore_handR_reposition_S(AnimationEventData& data) {
		auto& VoreData = Vore::GetSingleton().GetVoreData(&data.giant);
		VoreData.Swallow();
		auto giant = &data.giant;
		AdjustFacialExpression(giant, 0, 0.0, "modifier"); // blink L
		AdjustFacialExpression(giant, 1, 0.0, "modifier"); // blink R
		if (!AllowDevourment()) {
			Runtime::PlaySoundAtNode("VoreSwallow", giant, 1.0, 1.0, "NPC Head [Head]"); // Play sound
		}
		StartRHandRumble("HandR", data.giant, 0.75, 0.15);
	}

	void GTSvore_handL_reposition_S(AnimationEventData& data) {
		StartLHandRumble("HandL", data.giant, 0.75, 0.15);
	}

	void GTSvore_handR_reposition_E(AnimationEventData& data) {
		StopRHandRumble("HandR", data.giant);
	}

	void GTSvore_handL_reposition_E(AnimationEventData& data) {
		StopLHandRumble("HandL", data.giant);
	}

	void GTSvore_eat_actor(AnimationEventData& data) {
		auto& VoreData = Vore::GetSingleton().GetVoreData(&data.giant);
		AdjustFacialExpression(&data.giant, 2, 0.0, "expression"); // Remove smile
		VoreData.KillAll();
	}

	void GTSvore_detachactor_AnimObject_A(AnimationEventData& data) {
	}

	void GTSvore_standup_start(AnimationEventData& data) {
		auto giant = &data.giant;
		StartBodyRumble("BodyRumble", data.giant, 0.35, 0.10, false);
		if (!Runtime::GetBool("FreeLookOnVore") && giant->formID == 0x14) {
			ManageCamera(giant, false, 2.0);
		}
	}

	void GTSvore_impactRS(AnimationEventData& data) {
		GRumble::Once("StompRS", &data.giant, 0.90, 0.10, RNode);
		float perk = GetPerkBonus_Basics(&data.giant);
		DoFootstepSound(&data.giant, 0.90, FootEvent::Right, RNode);
		DoDustExplosion(&data.giant, 0.90, FootEvent::Right, RNode);
		DoDamageEffect(&data.giant, 1.6, 1.3, 30, 0.25, FootEvent::Right, 1.0, DamageSource::CrushedRight);
		DoLaunch(&data.giant, 0.60 * perk, 0.80 * data.animSpeed, FootEvent::Right);
	}

	void GTSvore_standup_end(AnimationEventData& data) {
		auto giant = &data.giant;
		auto& VoreData = Vore::GetSingleton().GetVoreData(&data.giant);
		VoreData.ReleaseAll();
		if (Runtime::GetBool("FreeLookOnVore") && giant->formID == 0x14) {
			EnableFreeCamera();
		}
		GRumble::Stop("BodyRumble", &data.giant);
		ToggleEmotionEdit(giant, false);
	}
}


namespace Gts
{
	void Animation_VoreStanding::RegisterEvents() {
		AnimationManager::RegisterEvent("GTSvore_attachactor_AnimObject_A", "Vore", GTSvore_attachactor_AnimObject_A);
		AnimationManager::RegisterEvent("GTSvore_detachactor_AnimObject_A", "Vore", GTSvore_detachactor_AnimObject_A);
		AnimationManager::RegisterEvent("GTSvore_handR_reposition_S", "Vore", GTSvore_handR_reposition_S);
		AnimationManager::RegisterEvent("GTSvore_handL_reposition_S", "Vore", GTSvore_handL_reposition_S);
		AnimationManager::RegisterEvent("GTSvore_handR_reposition_E", "Vore", GTSvore_handR_reposition_E);
		AnimationManager::RegisterEvent("GTSvore_handL_reposition_E", "Vore", GTSvore_handL_reposition_E);
		AnimationManager::RegisterEvent("GTSvore_bringactor_start", "Vore", GTSvore_bringactor_start);
		AnimationManager::RegisterEvent("GTSvore_bringactor_end", "Vore", GTSvore_bringactor_end);
		AnimationManager::RegisterEvent("GTSvore_swallow_sound", "Vore", GTSvore_swallow_sound);
		AnimationManager::RegisterEvent("GTSvore_standup_start", "Vore", GTSvore_standup_start);
		AnimationManager::RegisterEvent("GTSvore_hand_extend", "Vore", GTSvore_hand_extend);
		AnimationManager::RegisterEvent("GTSvore_close_mouth", "Vore", GTSvore_close_mouth);
		AnimationManager::RegisterEvent("GTSvore_standup_end", "Vore", GTSvore_standup_end);
		AnimationManager::RegisterEvent("GTSvore_open_mouth", "Vore", GTSvore_open_mouth);
		AnimationManager::RegisterEvent("GTSvore_hand_grab", "Vore", GTSvore_hand_grab);
		AnimationManager::RegisterEvent("GTSvore_sit_start", "Vore", GTSvore_sit_start);
		AnimationManager::RegisterEvent("GTSvore_eat_actor", "Vore", GTSvore_eat_actor);
		AnimationManager::RegisterEvent("GTSvore_impactRS", "Vore", GTSvore_impactRS);
		AnimationManager::RegisterEvent("GTSvore_impactLS", "Vore", GTSvore_impactLS);
		AnimationManager::RegisterEvent("GTSvore_sit_end", "Vore", GTSvore_sit_end);
		AnimationManager::RegisterEvent("GTSvore_swallow", "Vore", GTSvore_swallow);
	}

	void Animation_VoreStanding::RegisterTriggers() {
		AnimationManager::RegisterTrigger("StartVore", "Vore", "GTSBEH_StartVore");
	}
}
