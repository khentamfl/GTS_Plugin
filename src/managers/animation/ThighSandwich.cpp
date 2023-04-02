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
#include "managers/animation/AnimationManager.hpp"
#include "managers/ThighSandwichController.hpp"
#include "managers/animation/ThighSandwich.hpp"
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
	void GTSSandwich_MoveLL_start(AnimationEventData& data) {
		auto giant = &data.giant;
		auto& sandwichdata = ThighSandwichController::GetSingleton().GetSandwichingData(giant);
		sandwichdata.EnableSuffocate(true);
	}
	void GTSSandwich_ThighImpact(AnimationEventData& data) {
		auto giant = &data.giant;
		auto& sandwichdata = ThighSandwichController::GetSingleton().GetSandwichingData(giant);
		Runtime::PlaySoundAtNode("ThighSandwichImpact", giant, 1.0, 1.0, "AnimObjectB");
		sandwichdata.EnableSuffocate(false);
		for (auto prey: sandwichdata.GetActors()) {
			float sizedifference = get_visual_scale(giant)/get_visual_scale(prey);
			float damage = 3.0 * sizedifference;
			DamageAV(prey, ActorValue::kHealth, damage);
			float hp = GetAV(prey, ActorValue::kHealth);
			
			if (damage > hp) {
				CrushManager::GetSingleton().Crush(giant, prey);
				sandwichdata.Remove(prey);
			}
		}
	}

	void GTSSandwich_DropDown(AnimationEventData& data) {
		auto& sandwichdata = ThighSandwichController::GetSingleton().GetSandwichingData(&data.giant);
		sandwichdata.ReleaseAll();
	}

	void ThighSandwichEnterEvent(const InputEventData& data) {
		auto pred = PlayerCharacter::GetSingleton();
		auto& Sandwiching = ThighSandwichController::GetSingleton();
		std::size_t numberOfPrey = 1;
		if (Runtime::HasPerk(pred, "MassVorePerk")) {
			numberOfPrey = 1 + (get_visual_scale(pred)/3);
		}
		std::vector<Actor*> preys = Sandwiching.GetSandwichTargetsInFront(pred, numberOfPrey);
		for (auto prey: preys) {
			Sandwiching.StartSandwiching(pred, prey);
		}
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
		InputManager::RegisterInputEvent("ThighSandwichEnter", ThighSandwichEnterEvent);
		InputManager::RegisterInputEvent("ThighSandwichAttack", ThighSandwichAttackEvent);
		InputManager::RegisterInputEvent("ThighSandwichExit", ThighSandwichExitEvent);
		AnimationManager::RegisterEvent("GTSSandwich_ThighImpact", "ThighSandwich", GTSSandwich_ThighImpact);
		AnimationManager::RegisterEvent("GTSSandwich_DropDown", "ThighSandwich", GTSSandwich_DropDown);
		AnimationManager::RegisterEvent("GTSSandwich_MoveLL_start", "ThighSandwich", GTSSandwich_MoveLL_start);
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
}