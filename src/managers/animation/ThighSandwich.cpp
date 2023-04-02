// Animation: Stomp
//  - Stages
/*
GTSSandwich_EnterAnim           // Animation was just fired
GTSSandwich_MoveBody_start      // Rumble for entire body
GTSSandwich_EnableRune          // Trigger the ture
GTSSandwich_SitStart            // When sit start happens
GTSSandwich_MoveBody_end        // Stop them (When body pretty much enters 'idle' state)
GTSSandwich_MoveLL_start        // Left leg starts to move in space (When starting to crush)
GTSSandwich_ThighImpact         // When Both thighs are supposed to deal damage to an actor (When 'Sandwiching') happens
GTSSandwich_MoveLL_end          // Left leg isn't moving anymore (These 2 should be repeated)
GTSSandwich_ThighLoop_Enter     // Enter Thigh Idle Loop

GTSSandwich_ThighAttack_start   // When we trigger Thigh Attack

GTSSandwich_ThighLoop_Exit      // Exit thigh idle loop
GTSSandwich_DisableRune         // Remove Rune
GTSSandwich_DropDown            // When actor starts to 'jump off' from Rune
GTSSandwich_FootImpact          // When both feet collide with the ground
GTSSandwich_ExitAnim            // Animation is over

GTSBEH_ThighSandwich_Start
GTSBEH_ThighSandwich_Attack
GTSBEH_ThighSandwich_ExitLoop

//AnimObjectA = Tiny
//AnimObjectB = rune
*/

#include "managers/animation/ThighSandwich.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/InputManager.hpp"
#include "managers/CrushManager.hpp"
#include "managers/explosion.hpp"
#include "managers/footstep.hpp"
#include "managers/tremor.hpp"
#include "managers/Rumble.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "spring.hpp"
#include "node.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace {
	void ThighSandwichEnterEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		AnimationManager::StartAnim("ThighEnter", player);
	}
	void ThighSandwichAttackEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		AnimationManager::StartAnim("ThighAttack", player);
	}

	void ThighSandwichExitEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		AnimationManager::StartAnim("ThighExit", player);
	}
}

namespace Gts
{
	void AnimationThighSandwich::RegisterEvents() {
		/*AnimationManager::RegisterEvent("GTSstompimpactR", "Stomp", GTSstompimpactR);
		AnimationManager::RegisterEvent("GTSstompimpactL", "Stomp", GTSstompimpactL);
		AnimationManager::RegisterEvent("GTSstomplandR", "Stomp", GTSstomplandR);
		AnimationManager::RegisterEvent("GTSstomplandL", "Stomp", GTSstomplandL);
		AnimationManager::RegisterEvent("GTSstompstartR", "Stomp", GTSstompstartR);
		AnimationManager::RegisterEvent("GTSstompstartL", "Stomp", GTSstompstartL);
		AnimationManager::RegisterEvent("GTSStompendR", "Stomp", GTSStompendR);
		AnimationManager::RegisterEvent("GTSStompendL", "Stomp", GTSStompendL);
		AnimationManager::RegisterEvent("GTS_Next", "Stomp", GTS_Next);
		AnimationManager::RegisterEvent("GTSBEH_Exit", "Stomp", GTSBEH_Exit);*/

		InputManager::RegisterInputEvent("ThighSandwichEnter", ThighSandwichEnterEvent);
		InputManager::RegisterInputEvent("ThighSandwichAttack", ThighSandwichAttackEvent);
		InputManager::RegisterInputEvent("ThighSandwichExit", ThighSandwichExitEvent);
	}

	void AnimationThighSandwich::RegisterTriggers() {
		AnimationManager::RegisterTrigger("ThighEnter", "ThighSandwich", "GTSBEH_ThighSandwich_Start");
		AnimationManager::RegisterTrigger("ThighAttack", "ThighSandwich", "GTSBEH_ThighSandwich_Attack");
		AnimationManager::RegisterTrigger("ThighExit", "ThighSandwich", "GTSBEH_ThighSandwich_ExitLoop");
	}

	AnimationThighSandwich& AnimationThighSandwich::GetSingleton() noexcept {
		static AnimationThighSandwich instance;
		return instance;
	}

	std::string AnimationThighSandwich::DebugName() {
		return "AnimationThighSandwich";
	}

	void AnimationThighSandwich::Update() {

	}


	void AnimationThighSandwich::GrabActor(Actor* giant, TESObjectREFR* tiny) {
		AnimationThighSandwich::GetSingleton().data.try_emplace(giant, tiny, 1.0);
	}

	void AnimationThighSandwich::Release(Actor* giant) {
		AnimationThighSandwich::GetSingleton().data.erase(giant);
	}

	TESObjectREFR* AnimationThighSandwich::GetHeldObj(Actor* giant) {
		try {
			auto& me = AnimationThighSandwich::GetSingleton();
			return me.data.at(giant).tiny;
		} catch (std::out_of_range e) {
			return nullptr;
		}

	}
	Actor* AnimationThighSandwich::GetHeldActor(Actor* giant) {
		auto obj = AnimationThighSandwich::GetHeldObj(giant);
		Actor* actor = skyrim_cast<Actor*>(obj);
		if (actor) {
			return actor;
		} else {
			return nullptr;
		}
	}

	ThighData::ThighData(TESObjectREFR* tiny, float target) : 
		tiny(tiny), 
		target(target),
		RuneController(Spring(1.0, 0.5))
		{
	}
}