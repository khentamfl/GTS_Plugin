#include "managers/animation/ActorVore.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/CrushManager.hpp"
#include "utils/actorUtils.hpp"
#include "managers/Rumble.hpp"
#include "managers/vore.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"

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

	void AdjustFacialExpression(Actor* giant, std::uint32_t ph, float power, std::string_view type) {
		if (giant) {
			auto fgen = giant->GetFaceGenAnimationData();
			if (fgen) {
				if (type == "phenome") {
					fgen->phenomeKeyFrame.SetValue(ph, power);
				} if (type == "expression") {
					fgen->exprOverride = false;
					fgen->SetExpressionOverride(ph, power);
					fgen->expressionKeyFrame.SetValue(ph, power); // Expression doesn't need Spring since it is already smooth by default
					fgen->exprOverride = true;
				} if (type == "modifier") {
					fgen->modifierKeyFrame.SetValue(ph, power);
					fgen->customKeyFrame.SetValue(ph, power);
				}
			}
		}
	}

	void GTSvore_sit_start(AnimationEventData& data) {
		auto giant = &data.giant;
		if (Runtime::GetBool("FreeLookOnVore") && giant->formID == 0x14) {
			EnableFreeCamera();
		}
	}

	void GTSvore_impactLS(AnimationEventData& data) {
		float scale = get_visual_scale(&data.giant);
		float volume = scale * 0.20 * (data.animSpeed * data.animSpeed);
		Runtime::PlaySoundAtNode(LSound, &data.giant, volume, 1.0, LNode);
		//Rumble::Start("StompL", &data.giant, 0.25, RNode);
	}

	void GTSvore_sit_end(AnimationEventData& data) {
	}

	void GTSvore_hand_extend(AnimationEventData& data) {
	}

	void GTSvore_hand_grab(AnimationEventData& data) {
		auto giant = &data.giant;
		auto& VoreData = Vore::GetSingleton().GetVoreData(giant);
		VoreData.GrabAll();
		AdjustFacialExpression(giant, 2, 1.0, "expression"); // smile (expression)
		if (!Runtime::GetBool("FreeLookOnVore") && giant->formID == 0x14) {
			for (auto& tiny: VoreData.GetVories()) {
				PlayerCamera::GetSingleton()->cameraTarget = tiny->CreateRefHandle();
			}
		}
	}

	void GTSvore_attachactor_AnimObject_A(AnimationEventData& data) {
	}

	void GTSvore_bringactor_start(AnimationEventData& data) {
		AdjustFacialExpression(&data.giant, 5, 0.8, "phenome"); // Smile a bit (Mouth)
	}

	void GTSvore_open_mouth(AnimationEventData& data) {
		auto giant = &data.giant;
		auto& VoreData = Vore::GetSingleton().GetVoreData(giant);
		AdjustFacialExpression(giant, 0, 1.0, "phenome"); // Start opening mouth
		AdjustFacialExpression(giant, 1, 0.5, "phenome"); // Open it wider
		AdjustFacialExpression(giant, 0, 0.80, "modifier"); // blink L
		AdjustFacialExpression(giant, 1, 0.80, "modifier"); // blink R
		VoreData.EnableMouthShrinkZone(true);
	}

	void GTSvore_bringactor_end(AnimationEventData& data) {
		auto giant = &data.giant;
		auto& VoreData = Vore::GetSingleton().GetVoreData(&data.giant);
	}

	void GTSvore_swallow(AnimationEventData& data) {
	}

	void GTSvore_swallow_sound(AnimationEventData& data) {
		AdjustFacialExpression(&data.giant, 5, 0.0, "phenome"); // Remove smile (Mouth)
	}

	void GTSvore_close_mouth(AnimationEventData& data) {
		auto giant = &data.giant;
		auto& VoreData = Vore::GetSingleton().GetVoreData(giant);

		VoreData.EnableMouthShrinkZone(false);
		AdjustFacialExpression(giant, 0, 0.0, "phenome"); // Close mouth
		AdjustFacialExpression(giant, 1, 0.0, "phenome"); // Close mouth
	}

	void GTSvore_handR_reposition_S(AnimationEventData& data) {
		auto& VoreData = Vore::GetSingleton().GetVoreData(&data.giant);
		VoreData.Swallow();
		auto giant = &data.giant;
		AdjustFacialExpression(giant, 0, 0.0, "modifier"); // blink L
		AdjustFacialExpression(giant, 1, 0.0, "modifier"); // blink R
		Runtime::PlaySoundAtNode("VoreSwallow", giant, 1.0, 1.0, "NPC Head [Head]"); // Play sound
	}

	void GTSvore_handL_reposition_S(AnimationEventData& data) {
	}

	void GTSvore_handR_reposition_E(AnimationEventData& data) {
	}

	void GTSvore_handL_reposition_E(AnimationEventData& data) {
	}

	void GTSvore_eat_actor(AnimationEventData& data) {
		auto& VoreData = Vore::GetSingleton().GetVoreData(&data.giant);
		AdjustFacialExpression(&data.giant, 2, 0.0, "expression"); // Remove smile
		VoreData.KillAll();
		VoreData.ReleaseAll();
	}

	void GTSvore_detachactor_AnimObject_A(AnimationEventData& data) {
	}

	void GTSvore_standup_start(AnimationEventData& data) {
		auto giant = &data.giant;
		if (!Runtime::GetBool("FreeLookOnVore") && giant->formID == 0x14) {
			PlayerCamera::GetSingleton()->cameraTarget = PlayerCharacter::GetSingleton()->CreateRefHandle();
		}
		
	}

	void GTSvore_impactRS(AnimationEventData& data) {
	}

	void GTSvore_standup_end(AnimationEventData& data) {
		auto giant = &data.giant;
		if (Runtime::GetBool("FreeLookOnVore") && giant->formID == 0x14) {
			EnableFreeCamera();
		}
	}

}


namespace Gts
{
	void AnimationActorVore::RegisterEvents() {
		AnimationManager::RegisterEvent("GTSvore_attachactor_AnimObject_A", "ActorVore", GTSvore_attachactor_AnimObject_A);
		AnimationManager::RegisterEvent("GTSvore_detachactor_AnimObject_A", "ActorVore", GTSvore_detachactor_AnimObject_A);
		AnimationManager::RegisterEvent("GTSvore_handR_reposition_S", "ActorVore", GTSvore_handR_reposition_S);
		AnimationManager::RegisterEvent("GTSvore_handL_reposition_S", "ActorVore", GTSvore_handL_reposition_S);
		AnimationManager::RegisterEvent("GTSvore_handR_reposition_E", "ActorVore", GTSvore_handR_reposition_E);
		AnimationManager::RegisterEvent("GTSvore_handL_reposition_E", "ActorVore", GTSvore_handL_reposition_E);
		AnimationManager::RegisterEvent("GTSvore_bringactor_start", "ActorVore", GTSvore_bringactor_start);
		AnimationManager::RegisterEvent("GTSvore_bringactor_end", "ActorVore", GTSvore_bringactor_end);
		AnimationManager::RegisterEvent("GTSvore_swallow_sound", "ActorVore", GTSvore_swallow_sound);
		AnimationManager::RegisterEvent("GTSvore_standup_start", "ActorVore", GTSvore_standup_start);
		AnimationManager::RegisterEvent("GTSvore_hand_extend", "ActorVore", GTSvore_hand_extend);
		AnimationManager::RegisterEvent("GTSvore_close_mouth", "ActorVore", GTSvore_close_mouth);
		AnimationManager::RegisterEvent("GTSvore_standup_end", "ActorVore", GTSvore_standup_end);
		AnimationManager::RegisterEvent("GTSvore_open_mouth", "ActorVore", GTSvore_open_mouth);
		AnimationManager::RegisterEvent("GTSvore_hand_grab", "ActorVore", GTSvore_hand_grab);
		AnimationManager::RegisterEvent("GTSvore_sit_start", "ActorVore", GTSvore_sit_start);
		AnimationManager::RegisterEvent("GTSvore_eat_actor", "ActorVore", GTSvore_eat_actor);
		AnimationManager::RegisterEvent("GTSvore_impactRS", "ActorVore", GTSvore_impactRS);
		AnimationManager::RegisterEvent("GTSvore_impactLS", "ActorVore", GTSvore_impactLS);
		AnimationManager::RegisterEvent("GTSvore_sit_end", "ActorVore", GTSvore_sit_end);
		AnimationManager::RegisterEvent("GTSvore_swallow", "ActorVore", GTSvore_swallow);
	}

	void AnimationActorVore::RegisterTriggers() {
		AnimationManager::RegisterTrigger("StartVore", "ActorVore", "GTSBEH_StartVore");
	}
}
