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
#include "managers/CrushManager.hpp"
#include "managers/Rumble.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace {
	const std::string_view RNode = "NPC R Foot [Rft ]";
	const std::string_view LNode = "NPC L Foot [Lft ]";
	const std::string_view RSound = "lFootstepR";
	const std::string_view LSound = "lFootstepL";

	void ResetAnimationSpeed(Actor* actor) {
		AnimationManager::GetSingleton().ResetAnimSpeed(actor);
	}

	void GTSstompimpactR(AnimationEventData& data) {
		data.stage = 1;
		float scale = get_visual_scale(&data.giant);
		float volume = scale * 0.20 * (data.animSpeed * data.animSpeed);

		Runtime::PlaySoundAtNode(RSound, &data.giant, volume, 1.0, RNode);
		Rumble::Once("StompR", &data.giant, volume * 8, RNode);
		ConsoleLog::GetSingleton()->Print("Stomp: GTSstompimpactR, volume: %g", volume);
	}

	void GTSstompimpactL(AnimationEventData& data) {
		data.stage = 1;
		float scale = get_visual_scale(&data.giant);
		float volume = scale * 0.20 * (data.animSpeed * data.animSpeed);

		Runtime::PlaySoundAtNode(LSound, &data.giant, volume, 1.0, LNode);
		Rumble::Once("StompL", &data.giant, volume * 6, LNode);

		ConsoleLog::GetSingleton()->Print("Stomp: GTSstompimpactL");
	}

	void GTSstomplandR(AnimationEventData& data) {
		data.stage = 2;
		float scale = get_visual_scale(&data.giant);
		float volume = scale * 0.20 * (data.animSpeed * data.animSpeed);

		Runtime::PlaySoundAtNode(RSound, &data.giant, volume, 1.0, RNode);
		Rumble::Start("StompR", &data.giant, 0.25, RNode);
		ConsoleLog::GetSingleton()->Print("Stomp: GTSstomplandR");
	}

	void GTSstomplandL(AnimationEventData& data) {
		data.stage = 2;
		float scale = get_visual_scale(&data.giant);
		float volume = scale * 0.20 * (data.animSpeed * data.animSpeed);

		Runtime::PlaySoundAtNode(LSound, &data.giant, volume, 1.0, LNode);
		Rumble::Start("StompL", &data.giant, 0.25, RNode);
		ConsoleLog::GetSingleton()->Print("Stomp: GTSstomplandL");
	}


	void GTSstompstartR(AnimationEventData& data) {
		data.stage = 3;
		data.canEditAnimSpeed = true;
		Rumble::Start("StompR", &data.giant, 0.35, RNode);
		ConsoleLog::GetSingleton()->Print("Stomp: GTSstompstartR");
	}

	void GTSstompstartL(AnimationEventData& data) {
		data.stage = 3;
		data.canEditAnimSpeed = true;
		Rumble::Start("StompL", &data.giant, 0.35, LNode); // Start stonger effect
		ConsoleLog::GetSingleton()->Print("Stomp: GTSstompstartL");
	}

	void GTSStompendR(AnimationEventData& data) {
		data.stage = 4;
		data.canEditAnimSpeed = false;
		data.animSpeed = 1.0;
		ResetAnimationSpeed(&data.giant);
		ConsoleLog::GetSingleton()->Print("Stomp: GTSstompendR");
	}

	void GTSStompendL(AnimationEventData& data) {
		data.stage = 4;
		data.canEditAnimSpeed = false;
		data.animSpeed = 1.0;
		ResetAnimationSpeed(&data.giant);
		ConsoleLog::GetSingleton()->Print("Stomp: GTSstompendL");
	}

	void GTS_Next(AnimationEventData& data) {
		data.stage = 0;
		Rumble::Stop("StompR", &data.giant);
		Rumble::Stop("StompL", &data.giant);
		ConsoleLog::GetSingleton()->Print("Stomp: GTS_next");
	}

	void GTSBEH_Exit(AnimationEventData& data) {
		data.stage = 0;
		Rumble::Stop("StompR", &data.giant);
		Rumble::Stop("StompL", &data.giant);
		ConsoleLog::GetSingleton()->Print("Stomp: GTSBEH_Exit");
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
	}

	void AnimationStomp::RegisterTriggers() {
		AnimationManager::RegisterTrigger("StompRight", "Stomp", "GtsModStompAnimRight");
		AnimationManager::RegisterTrigger("StompLeft", "Stomp", "GtsModStompAnimLeft");
	}
}
