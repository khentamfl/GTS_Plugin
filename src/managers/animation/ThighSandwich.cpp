// Animation: Stomp
//  - Stages
/*
GTSSandwich_EnterAnim           // Animation was just fired
GTSSandwich_MoveBody_start      // Rumble for entire body
GTSSandwich_EnableRune          // Trigger the rune
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
#include "utils/actorUtils.hpp"
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
	const std::string_view RNode = "NPC R Foot [Rft ]";
	const std::string_view LNode = "NPC L Foot [Lft ]";

	void PrintThighKill(Actor* pred, Actor* prey) {
        int random = rand() % 3;
        if (random <= 1) {
			ConsoleLog::GetSingleton()->Print("%s was crushed by the thighs of %s", prey->GetDisplayFullName(), pred->GetDisplayFullName());
		} else if (random == 2) {
			ConsoleLog::GetSingleton()->Print("Thighs of %s gently crushed %s", pred->GetDisplayFullName(), prey->GetDisplayFullName());
		} else if (random == 3) {
			ConsoleLog::GetSingleton()->Print("%s was killed between the thighs of %s", prey->GetDisplayFullName(), pred->GetDisplayFullName());
		}
    }

	const std::vector<std::string_view> L_LEG_NODES = {
		"NPC L Foot [Lft ]",
		"NPC L Toe0 [LToe]",
		"NPC L Calf [LClf]",
		"NPC L PreRearCalf",
		"NPC L FrontThigh",
		"NPC L RearCalf [RrClf]",
	};

	void DoThighDamage(Actor* giant, actor* tiny, float animSpeed) {
		auto& sandwichdata = ThighSandwichController::GetSingleton().GetSandwichingData(giant);
		auto& sizemanager = SizeManager::GetSingleton();
		float sizedifference = get_visual_scale(giant)/get_visual_scale(tiny);
		float additionaldamage = 1.0 + sizemanager.GetSizeVulnerability(tiny); // Get size damage debuff from enemy
		float normaldamage = std::clamp(sizemanager.GetSizeAttribute(giant, 0) * 0.25, 0.25, 999.0);
		float damage = 2.0 * sizedifference * animSpeed;
		DamageAV(tiny, ActorValue::kHealth, damage);
		float hp = GetAV(tiny, ActorValue::kHealth);
		if (damage > hp && sizedifference >= 8.0) {
			CrushManager::GetSingleton().Crush(giant, tiny);
			PrintThighKill(giant, tiny);
			sandwichdata.Remove(tiny);
		}
	}

	

	void StartLeftLegRumble(std::string_view tag, Actor& actor, float power, float halflife) {
		for (auto& node_name: L_LEG_NODES) {
			std::string rumbleName = std::format("{}{}", tag, node_name);
			Rumble::Start(rumbleName, &actor, power,  halflife, node_name);
		}
	}

	void StopLeftLegRumble(std::string_view tag, Actor& actor) {
		for (auto& node_name: L_LEG_NODES) {
			std::string rumbleName = std::format("{}{}", tag, node_name);
			Rumble::Stop(rumbleName, &actor);
		}
	}

	void GTSSandwich_EnterAnim(AnimationEventData& data) {
	}
	void GTSSandwich_MoveBody_start(AnimationEventData& data) {
	}
	void GTSSandwich_EnableRune(AnimationEventData& data) {
		auto& sandwichdata = ThighSandwichController::GetSingleton().GetSandwichingData(&data.giant);
		sandwichdata.RuneScale = true;
	}
	void GTSSandwich_SitStart(AnimationEventData& data) {
	}
	void GTSSandwich_MoveBody_end(AnimationEventData& data) {
	}
	void GTSSandwich_MoveLL_start(AnimationEventData& data) { 
		data.stage = 1.0;
		data.canEditAnimSpeed = true;
		data.animSpeed = 1.66;
		auto& sandwichdata = ThighSandwichController::GetSingleton().GetSandwichingData(&data.giant);
		sandwichdata.EnableSuffocate(false);
		StartLeftLegRumble("LLSandwich", data.giant, 0.5, 0.12);
	}

	void GTSSandwich_ThighImpact(AnimationEventData& data) {
		auto& sandwichdata = ThighSandwichController::GetSingleton().GetSandwichingData(&data.giant);
		Runtime::PlaySoundAtNode("ThighSandwichImpact", &data.giant, 1.0, 1.0, "AnimObjectB");
		sandwichdata.EnableSuffocate(true);
		Rumble::Once("ThighImpact", &data.giant, 1.8, 0.15, "AnimObjectA");
		for (auto tiny: sandwichdata.GetActors()) {
			DoThighDamage(&data.giant, tiny, data.animSpeed);
			tiny->NotifyAnimationGraph("ragdoll");
		}
	}

	void GTSSandwich_MoveLL_end(AnimationEventData& data) {
		data.canEditAnimSpeed = false;
		data.animSpeed = 1.0;
		StopLeftLegRumble("LLSandwich", data.giant);
	}

	void GTSSandwich_ThighLoop_Enter(AnimationEventData& data) {
	}
	void GTSSandwich_ThighAttack_start(AnimationEventData& data) {
	}



	void GTSSandwich_DropDown(AnimationEventData& data) {
		auto& sandwichdata = ThighSandwichController::GetSingleton().GetSandwichingData(&data.giant);
		sandwichdata.ReleaseAll();
	}

	void GTSSandwich_FootImpact(AnimationEventData& data) {
		DoSizeEffect(&data.giant, 0.75, FootEvent::Right, RNode);
		DoSizeEffect(&data.giant, 0.75, FootEvent::Left, LNode);
		DoDamageEffect(&data.giant, 4.0, 1.6, 20);
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

	void ThighSandwichHeavyAttackEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		AnimationManager::StartAnim("ThighAttack_Heavy", player);
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
		InputManager::RegisterInputEvent("ThighSandwichAttackHeavy", ThighSandwichHeavyAttackEvent);
		InputManager::RegisterInputEvent("ThighSandwichExit", ThighSandwichExitEvent);
		AnimationManager::RegisterEvent("GTSSandwich_ThighImpact", "ThighSandwich", GTSSandwich_ThighImpact);
		AnimationManager::RegisterEvent("GTSSandwich_DropDown", "ThighSandwich", GTSSandwich_DropDown);
		AnimationManager::RegisterEvent("GTSSandwich_MoveLL_start", "ThighSandwich", GTSSandwich_MoveLL_start);
		AnimationManager::RegisterEvent("GTSSandwich_MoveLL_end", "ThighSandwich", GTSSandwich_MoveLL_end);
		AnimationManager::RegisterEvent("GTSSandwich_EnterAnim", "ThighSandwich", GTSSandwich_EnterAnim);
		AnimationManager::RegisterEvent("GTSSandwich_MoveBody_start", "ThighSandwich", GTSSandwich_MoveBody_start);
		AnimationManager::RegisterEvent("GTSSandwich_EnableRune", "ThighSandwich", GTSSandwich_EnableRune);
		AnimationManager::RegisterEvent("GTSSandwich_SitStart", "ThighSandwich", GTSSandwich_SitStart);
		AnimationManager::RegisterEvent("GTSSandwich_MoveBody_end", "ThighSandwich", GTSSandwich_MoveBody_end);
		AnimationManager::RegisterEvent("GTSSandwich_ThighLoop_Enter", "ThighSandwich", GTSSandwich_ThighLoop_Enter);
		AnimationManager::RegisterEvent("GTSSandwich_FootImpact", "ThighSandwich", GTSSandwich_FootImpact);
	}

	void AnimationThighSandwich::RegisterTriggers() {
		AnimationManager::RegisterTrigger("ThighEnter", "ThighSandwich", "GTSBEH_ThighSandwich_Start");
		AnimationManager::RegisterTrigger("ThighAttack", "ThighSandwich", "GTSBEH_ThighSandwich_Attack");
		AnimationManager::RegisterTrigger("ThighAttack_Heavy", "ThighSandwich", "GTSBEH_ThighSandwich_Attack_Heavy");
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