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
#include "managers/Rumble.hpp"
#include "managers/tremor.hpp"
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

	void DoEffects(Actor* giant, float modifier, std::string_view node) {
		auto& tremor = TremorManager::GetSingleton();
		auto& explosion = ExplosionManager::GetSingleton();
		tremor.OnImpact_Manual(giant, modifier, node);
		explosion.OnImpact_Manual(giant, modifier, node);
	}

	void DoDamage(Actor* giant, float damage, float radius) {
		auto& sizemanager = SizeManager::GetSingleton();
		AccurateDamage::GetSingleton().DoAccurateCollision(giant, 12.0 * damage, 1.0);
	}

	void GTSstompimpactR(AnimationEventData& data) {
		float scale = get_visual_scale(&data.giant);
		float volume = scale * 0.20 * (data.animSpeed * data.animSpeed);

		Runtime::PlaySoundAtNode(RSound, &data.giant, volume, 1.0, RNode);
		//Rumble::Once("StompR", &data.giant, volume * 8, RNode);
		DoDamage(&data.giant, 1.0 * data.animSpeed, 1.0);
		DoEffects(&data.giant, 1.75 * data.animSpeed, RNode);
	}

	void GTSstompimpactL(AnimationEventData& data) {
		//data.stage = 1;
		float scale = get_visual_scale(&data.giant);
		float volume = scale * 0.20 * (data.animSpeed * data.animSpeed);

		Runtime::PlaySoundAtNode(LSound, &data.giant, volume, 1.0, LNode);
		//Rumble::Once("StompL", &data.giant, volume * 8, LNode);
		DoDamage(&data.giant, 1.0 * data.animSpeed, 1.0);
		DoEffects(&data.giant, 1.75 * data.animSpeed, LNode);

	}

	void GTSstomplandR(AnimationEventData& data) {
		//data.stage = 2;
		float scale = get_visual_scale(&data.giant);
		float volume = scale * 0.20 * (data.animSpeed * data.animSpeed);

		Runtime::PlaySoundAtNode(RSound, &data.giant, volume, 1.0, RNode);
		//Rumble::Start("StompR", &data.giant, 0.25, RNode);
		DoDamage(&data.giant, 0.6, 1.0);
		DoEffects(&data.giant, 0.85 * data.animSpeed, RNode);
	}

	void GTSstomplandL(AnimationEventData& data) {
		//data.stage = 2;
		float scale = get_visual_scale(&data.giant);
		float volume = scale * 0.20 * (data.animSpeed * data.animSpeed);

		Runtime::PlaySoundAtNode(LSound, &data.giant, volume, 1.0, LNode);
		//Rumble::Start("StompL", &data.giant, 0.25, RNode);
		DoDamage(&data.giant, 0.6, 1.0);
		DoEffects(&data.giant, 0.85 * data.animSpeed, LNode);
	}


	void GTSstompstartR(AnimationEventData& data) {
		data.canEditAnimSpeed = true;
		Rumble::Start("StompR", &data.giant, 0.35, RNode);
	}

	void GTSstompstartL(AnimationEventData& data) {
		data.canEditAnimSpeed = true;
		Rumble::Start("StompL", &data.giant, 0.35, LNode); // Start stonger effect
	}

	void GTSStompendR(AnimationEventData& data) {
		data.canEditAnimSpeed = false;
		data.animSpeed = 1.0;
	}

	void GTSStompendL(AnimationEventData& data) {
		data.canEditAnimSpeed = false;
		data.animSpeed = 1.0;
	}

	void GTS_Next(AnimationEventData& data) {
		Rumble::Stop("StompR", &data.giant);
		Rumble::Stop("StompL", &data.giant);
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
