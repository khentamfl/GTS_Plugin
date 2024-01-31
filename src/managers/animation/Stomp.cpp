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
#include "managers/damage/CollisionDamage.hpp"
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
	std::random_device rd;
	std::mt19937 e2(rd());

	std::vector<Actor*> Utils_findSquished(Actor* giant) {
		/*
		Find actor that are being pressed underfoot
		*/
		std::vector<Actor*> result = {};
		if (!giant) {
			return result;
		}
		float giantScale = get_visual_scale(giant);
		auto giantLoc = giant->GetPosition();
		for (auto tiny: find_actors()) {
			if (tiny) {
				float tinyScale = get_visual_scale(tiny) * GetSizeFromBoundingBox(tiny);
				float scaleRatio = giantScale / tinyScale;
				if (scaleRatio > 3.5) {
					// 3.5 times bigger
					auto tinyLoc = tiny->GetPosition();
					auto distance = (giantLoc - tinyLoc).Length();
					if (distance < giantScale * 70) {
						// nearish
						result.push_back(tiny);
					}
				}
			}
		}
		return result;
	}

	void Utils_move_under_Foot(Actor* giant, std::string_view node) {
		auto footNode = find_node(giant, RNode);
		if (footNode) {
			auto footPos = footNode->world.translate;
			for (auto tiny: Utils_findSquished(giant)) {
				std::uniform_real_distribution<> dist(-10, 10);
				float dx = dist(e2);
				float dy = dist(e2);
				auto randomOffset = NiPoint3(dx, dy, 0.0);
				tiny->SetPosition(footPos + randomOffset, true);
			}
		}
	}

///////////////////////////////////////////////////////////////////////////////////////////////////// Events

	void GTSstompstartR(AnimationEventData& data) {
		data.stage = 1;
		data.canEditAnimSpeed = true;
		data.animSpeed = 1.33;
		if (data.giant.formID != 0x14) {
			data.animSpeed = 1.33 + GetRandomBoost()/2;
		}
		DrainStamina(&data.giant, "StaminaDrain_Stomp", "DestructionBasics", true, 1.8);
		ManageCamera(&data.giant, true, 6.0);
		GRumble::Start("StompR", &data.giant, 0.35, 0.15, RNode);
		//log::info("StompStartR true");
	}

	void GTSstompstartL(AnimationEventData& data) {
		data.stage = 1;
		data.canEditAnimSpeed = true;
		data.animSpeed = 1.33;
		if (data.giant.formID != 0x14) {
			data.animSpeed = 1.33 + GetRandomBoost()/2;
		}
		DrainStamina(&data.giant, "StaminaDrain_Stomp", "DestructionBasics", true, 1.8);
		ManageCamera(&data.giant, true, 5.0);
		GRumble::Start("StompL", &data.giant, 0.45, 0.15, LNode); // Start stonger effect
		//log::info("StompStartL true");
	}

	void GTSstompimpactR(AnimationEventData& data) {
		float shake = 1.0;
		float launch = 1.0;
		float dust = 1.25;
		float perk = GetPerkBonus_Basics(&data.giant);
		if (HasSMT(&data.giant)) {
			shake = 4.0;
			launch = 1.2;
			dust = 1.45;
		}
		GRumble::Once("StompR", &data.giant, 2.20 * shake, 0.0, RNode);

		Utils_move_under_Foot(&data.giant, RNode);

		DoDamageEffect(&data.giant, (2.2 + data.animSpeed/8) * launch * perk, (1.45 + data.animSpeed/4) * launch, 10, 0.25, FootEvent::Right, 1.0, DamageSource::CrushedRight);
		DoFootstepSound(&data.giant, 1.0 + data.animSpeed/8, FootEvent::Right, RNode);
		DoDustExplosion(&data.giant, dust + (data.animSpeed * 0.05), FootEvent::Right, RNode);
		DoLaunch(&data.giant, 0.80 * perk, 1.35 * data.animSpeed, FootEvent::Right);
		DrainStamina(&data.giant, "StaminaDrain_Stomp", "DestructionBasics", false, 1.8);
		FootGrindCheck_Right(&data.giant, 1.45, false);
	}

	void GTSstompimpactL(AnimationEventData& data) {
		float shake = 1.0;
		float launch = 1.0;
		float dust = 1.25;
		float perk = GetPerkBonus_Basics(&data.giant);
		if (HasSMT(&data.giant)) {
			shake = 4.0;
			launch = 1.2;
			dust = 1.45;
		}
		GRumble::Once("StompL", &data.giant, 2.20 * shake, 0.0, LNode);

		Utils_move_under_Foot(&data.giant, LNode);

		DoDamageEffect(&data.giant, (2.2 + data.animSpeed/8) * launch * perk, (1.45 + data.animSpeed/4) * launch, 10, 0.25, FootEvent::Left, 1.0, DamageSource::CrushedLeft);
		DoFootstepSound(&data.giant, 1.0 + data.animSpeed/14, FootEvent::Left, LNode);
		DoDustExplosion(&data.giant, dust + (data.animSpeed * 0.05), FootEvent::Left, LNode);
		DoLaunch(&data.giant, 0.80 * perk, 1.35 * data.animSpeed, FootEvent::Left);
		DrainStamina(&data.giant, "StaminaDrain_Stomp", "DestructionBasics", false, 1.8);
		FootGrindCheck_Left(&data.giant, 1.45, false);
	}

	void GTSstomplandR(AnimationEventData& data) {
		//data.stage = 2;
		float shake = 1.0;
		float bonus = 1.0;
		float dust = 0.85;
		float perk = GetPerkBonus_Basics(&data.giant);
		if (HasSMT(&data.giant)) {
			bonus = 1.3;
			dust = 1.35;
			shake = 4.0;
		}
		GRumble::Once("StompRL", &data.giant, 1.25 * shake, 0.05, RNode);
		DoDamageEffect(&data.giant, 2.0 * perk, 1.45, 25, 0.25, FootEvent::Right, 1.0, DamageSource::CrushedRight);
		DoFootstepSound(&data.giant, 1.0 + data.animSpeed/14, FootEvent::Right, RNode);
		DoDustExplosion(&data.giant, dust + (data.animSpeed * 0.05), FootEvent::Right, RNode);
		DoLaunch(&data.giant, 0.75 * perk, 1.5 + data.animSpeed/4, FootEvent::Right);
	}

	void GTSstomplandL(AnimationEventData& data) {
		//data.stage = 2;
		float shake = 1.0;
		float bonus = 1.0;
		float dust = 0.85;
		float perk = GetPerkBonus_Basics(&data.giant);
		if (HasSMT(&data.giant)) {
			bonus = 1.3;
			dust = 1.35;
			shake = 4.0;
		}
		GRumble::Once("StompLL", &data.giant, 1.25 * shake, 0.05, LNode);
		DoDamageEffect(&data.giant, 2.0 * perk, 1.45, 25, 0.25, FootEvent::Left, 1.0, DamageSource::CrushedLeft);
		DoFootstepSound(&data.giant, 1.0 + data.animSpeed/14, FootEvent::Left, LNode);
		DoDustExplosion(&data.giant, dust + data.animSpeed/4, FootEvent::Left, LNode);
		DoLaunch(&data.giant, 0.75 * perk, 1.5 + data.animSpeed/4, FootEvent::Left);
	}

	void GTSStompendR(AnimationEventData& data) {
		data.stage = 0;
		data.canEditAnimSpeed = false;
		data.animSpeed = 1.0;
		//BlockFirstPerson(&data.giant, false);
	}

	void GTSStompendL(AnimationEventData& data) {
		data.stage = 0;
		data.canEditAnimSpeed = false;
		data.animSpeed = 1.0;
		//BlockFirstPerson(&data.giant, false);
	}

	void GTS_Next(AnimationEventData& data) {
		GRumble::Stop("StompR", &data.giant);
		GRumble::Stop("StompL", &data.giant);
		GRumble::Stop("StompRL", &data.giant);
		GRumble::Stop("StompLL", &data.giant);
	}

	void GTSBEH_Exit(AnimationEventData& data) {
		GRumble::Stop("StompR", &data.giant);
		GRumble::Stop("StompL", &data.giant);
		DrainStamina(&data.giant, "StaminaDrain_Stomp", "DestructionBasics", false, 1.8);
		DrainStamina(&data.giant, "StaminaDrain_StrongStomp", "DestructionBasics", false, 2.8);
		ManageCamera(&data.giant, false, 6.0);
		ManageCamera(&data.giant, false, 5.0);
	}

	void RightStompEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		if (!CanPerformAnimation(player, 1) || IsGtsBusy(player)) {
			return;
		}
		float WasteStamina = 25.0;
		if (Runtime::HasPerk(player, "DestructionBasics")) {
			WasteStamina *= 0.65;
		}
		if (GetAV(player, ActorValue::kStamina) > WasteStamina) {
			//BlockFirstPerson(player, true);
			AnimationManager::StartAnim("StompRight", player);
		} else {
			TiredSound(player, "You're too tired to perform stomp");
		}
	}

	void LeftStompEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		if (!CanPerformAnimation(player, 1) || IsGtsBusy(player)) {
			return;
		}
		float WasteStamina = 25.0;
		if (Runtime::HasPerk(player, "DestructionBasics")) {
			WasteStamina *= 0.65;
		}
		if (GetAV(player, ActorValue::kStamina) > WasteStamina) {
			//BlockFirstPerson(player, true);
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