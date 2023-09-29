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

	void StartDamageAt_L(Actor* actor, float power, float crush, float pushpower, std::string_view node) {
		std::string name = std::format("LegKick_{}", actor->formID);
		auto gianthandle = actor->CreateRefHandle();
		TaskManager::Run(name, [=](auto& progressData) {
			if (!gianthandle) {
				return false;
			}
			auto giant = gianthandle.get().get();
			auto Leg = find_node(giant, node);
			if (Leg) {
				DoDamageAtPoint_Cooldown(giant, 28, 80.0 * power, Leg, 10, 0.30, crush, pushpower, DamageSource::KickedLeft);
			}
			return true;
		});
	}

	void StartDamageAt_R(Actor* actor, float power, float crush, float pushpower, std::string_view node) {
		std::string name = std::format("LegKick_{}", actor->formID);
		auto gianthandle = actor->CreateRefHandle();
		TaskManager::Run(name, [=](auto& progressData) {
			if (!gianthandle) {
				return false;
			}
			auto giant = gianthandle.get().get();
			auto Leg = find_node(giant, node);
			if (Leg) {
				DoDamageAtPoint_Cooldown(giant, 28, 80.0 * power, Leg, 10, 0.30, crush, pushpower, DamageSource::KickedRight);
			}
			return true;
		});
	}

	void StopAllDamageAndStamina(Actor* actor) {
		std::string name = std::format("LegKick_{}", actor->formID);
		DrainStamina(actor, "StaminaDrain_StrongKick", "DestructionBasics", false, 2.0, 8.0);
		DrainStamina(actor, "StaminaDrain_Kick", "DestructionBasics", false, 2.0, 4.0);
		TaskManager::Cancel(name);
	}

	void GTS_Kick_Camera_On_R(AnimationEventData& data) {
		TrackFeet(&data.giant, 6.0, true);
	}
	void GTS_Kick_Camera_On_L(AnimationEventData& data) {
		TrackFeet(&data.giant, 5.0, true);
	}
	void GTS_Kick_Camera_Off_R(AnimationEventData& data) {
		TrackFeet(&data.giant, 6.0, false);
	}
	void GTS_Kick_Camera_Off_L(AnimationEventData& data) {
		TrackFeet(&data.giant, 5.0, false);
	}

	void GTS_Kick_SwingLeg_L(AnimationEventData& data) {
	}
	void GTS_Kick_SwingLeg_R(AnimationEventData& data) {
	}

	void GTS_Kick_Stomp_R(AnimationEventData& data) {
		DoDamageEffect(&data.giant, 1.40, 1.6, 10, 0.20, FootEvent::Right, 1.0, DamageSource::CrushedRight);
		DoFootstepSound(&data.giant, 1.0, FootEvent::Right, RNode);
		DoDustExplosion(&data.giant, 1.0, FootEvent::Right, RNode);
		DoLaunch(&data.giant, 0.85, 1.75, 1.4, FootEvent::Right, 0.85);
	}
	void GTS_Kick_Stomp_L(AnimationEventData& data) {
		DoDamageEffect(&data.giant, 1.40, 1.6, 10, 0.20, FootEvent::Left, 1.0, DamageSource::CrushedLeft);
		DoFootstepSound(&data.giant, 1.0, FootEvent::Left, LNode);
		DoDustExplosion(&data.giant, 1.0, FootEvent::Left, LNode);
		DoLaunch(&data.giant, 0.85, 1.75, 1.4, FootEvent::Left, 0.85);
	}

	void GTS_Kick_HitBox_On_R(AnimationEventData& data) {
		StartDamageAt_R(&data.giant, 0.7, 1.8, 0.50, "NPC R Toe0 [RToe]");
		DrainStamina(&data.giant, "StaminaDrain_StrongKick", "DestructionBasics", true, 2.0, 4.0);
	}
	void GTS_Kick_HitBox_On_L(AnimationEventData& data) {
		StartDamageAt_L(&data.giant, 0.7, 1.8, 0.50, "NPC L Toe0 [LToe]");
		DrainStamina(&data.giant, "StaminaDrain_StrongKick", "DestructionBasics", true, 2.0, 4.0);
	}
	void GTS_Kick_HitBox_Off_R(AnimationEventData& data) {
		StopAllDamageAndStamina(&data.giant);
	}
	void GTS_Kick_HitBox_Off_L(AnimationEventData& data) {
		StopAllDamageAndStamina(&data.giant);
	}

	void GTS_Kick_HitBox_Power_On_R(AnimationEventData& data) {
		StartDamageAt_R(&data.giant, 1.4, 1.8, 1.8, "NPC R Toe0 [RToe]");
		DrainStamina(&data.giant, "StaminaDrain_StrongKick", "DestructionBasics", true, 2.0, 8.0);
	}
	void GTS_Kick_HitBox_Power_On_L(AnimationEventData& data) {
		StartDamageAt_L(&data.giant, 1.4, 1.8, 1.8, "NPC L Toe0 [LToe]");
		DrainStamina(&data.giant, "StaminaDrain_StrongKick", "DestructionBasics", true, 2.0, 8.0);
	}
	void GTS_Kick_HitBox_Power_Off_R(AnimationEventData& data) {
		StopAllDamageAndStamina(&data.giant);
	}
	void GTS_Kick_HitBox_Power_Off_L(AnimationEventData& data) {
		StopAllDamageAndStamina(&data.giant);
	}



	// ======================================================================================
	//  Triggers
	// ======================================================================================
	void LightKickLeftEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		if (!player->IsSneaking() && !player->AsActorState()->IsSprinting()) {
			float WasteStamina = 35.0 * GetWasteMult(player);
			if (GetAV(player, ActorValue::kStamina) > WasteStamina) {
				AnimationManager::StartAnim("SwipeLight_Left", player);
			} else {
				TiredSound(player, "You're too tired for a kick");
			}
		}
	}

	void LightKickRightEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		if (!player->IsSneaking() && !player->AsActorState()->IsSprinting()) {
			float WasteStamina = 35.0 * GetWasteMult(player);
			if (GetAV(player, ActorValue::kStamina) > WasteStamina) {
				AnimationManager::StartAnim("SwipeLight_Right", player);
			} else {
				TiredSound(player, "You're too tired for a kick");
			}
		}
	}

	void HeavyKickLeftEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		if (!player->IsSneaking() && !player->AsActorState()->IsSprinting()) {
			float WasteStamina = 80.0 * GetWasteMult(player);
			if (GetAV(player, ActorValue::kStamina) > WasteStamina) {
				AnimationManager::StartAnim("SwipeHeavy_Left", player);
			} else {
				TiredSound(player, "You're too tired for a Kick");
			}
		}
	}
	void HeavyKickRightEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		if (!player->IsSneaking() && !player->AsActorState()->IsSprinting()) {
			float WasteStamina = 80.0 * GetWasteMult(player);
			if (GetAV(player, ActorValue::kStamina) > WasteStamina) {
				AnimationManager::StartAnim("SwipeHeavy_Right", player);
			} else {
				TiredSound(player, "You're too tired for a Kick");
			}
		}
	}
}

namespace Gts
{
	void AnimationKicks::RegisterEvents() {
		InputManager::RegisterInputEvent("LightKickLeft", LightKickLeftEvent);
		InputManager::RegisterInputEvent("LightKickRight", LightKickRightEvent);
		InputManager::RegisterInputEvent("HeavyKickLeft", HeavyKickLeftEvent);
		InputManager::RegisterInputEvent("HeavyKickRight", HeavyKickRightEvent);

		AnimationManager::RegisterEvent("GTS_Kick_Camera_On_R", "Kicks", GTS_Kick_Camera_On_R);
		AnimationManager::RegisterEvent("GTS_Kick_Camera_On_L", "Kicks", GTS_Kick_Camera_On_L);
		AnimationManager::RegisterEvent("GTS_Kick_Camera_Off_R", "Kicks", GTS_Kick_Camera_On_R);
		AnimationManager::RegisterEvent("GTS_Kick_Camera_Off_L", "Kicks", GTS_Kick_Camera_On_L);

		AnimationManager::RegisterEvent("GTS_Kick_SwingLeg_R", "Kicks", GTS_Kick_SwingLeg_R);
		AnimationManager::RegisterEvent("GTS_Kick_SwingLeg_L", "Kicks", GTS_Kick_SwingLeg_L);

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
}