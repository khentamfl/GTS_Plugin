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
		Rumble::Start("StompR", &data.giant, 0.35, 0.15, RNode);
		log::info("StompStartR true");
	}

	void GTSstompstartL(AnimationEventData& data) {
		data.stage = 1;
		data.canEditAnimSpeed = true;
		data.animSpeed = 1.33;
		Rumble::Start("StompL", &data.giant, 0.45, 0.15, LNode); // Start stonger effect
		log::info("StompStartL true");
	}

	void GTSstompimpactR(AnimationEventData& data) {
		Rumble::Once("StompR", &data.giant, 2.20, 0.0, RNode);
		DoDamageEffect(&data.giant, 1.0, 1.0, 10);
		DoSizeEffect(&data.giant, 1.10, FootEvent::Right, RNode);
	}

	void GTSstompimpactL(AnimationEventData& data) {
		//data.stage = 1;
		Rumble::Once("StompL", &data.giant, 2.20, 0.0, LNode);
		DoDamageEffect(&data.giant, 1.0, 1.0, 10);
		DoSizeEffect(&data.giant, 1.10, FootEvent::Left, LNode);
	}

	void GTSstomplandR(AnimationEventData& data) {
		//data.stage = 2;
		Rumble::Start("StompRL", &data.giant, 0.45, 0.10, RNode);
		DoDamageEffect(&data.giant, 0.6, 1.0, 25);
		DoSizeEffect(&data.giant, 0.85, FootEvent::Right, RNode);
	}

	void GTSstomplandL(AnimationEventData& data) {
		//data.stage = 2;
		Rumble::Start("StompLL", &data.giant, 0.45, 0.10, LNode);
		DoDamageEffect(&data.giant, 0.6, 1.0, 25);
		DoSizeEffect(&data.giant, 0.85, FootEvent::Left, LNode);
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
	}

	void RightStompEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		AnimationManager::StartAnim("StompRight", player);
		log::info("Stomp Right");
	}

	void LeftStompEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		AnimationManager::StartAnim("StompLeft", player);
		log::info("Stomp Left");
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
