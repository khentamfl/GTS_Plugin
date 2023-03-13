#include "managers/animation/AnimationManager.hpp"

using namespace RE;
using namespace Gts;
using namespace std;

namespace {

	const std::vector<std::string_view> Anim_Vore = {
		"GTSvore_sitstart",         // [0] Start air rumble and camera shake
		"GTSvore_sitend",           // [1] Sit down completed
		"GTSvore_handextend",       // [2] Hand starts to move in space
		"GTSvore_handgrab",         // [3] Hand reached someone, grab actor
		"GTSvore_bringactorstart",  // [4] Hand brings someone to mouth
		"GTSvore_bringactorend",    // [5] Hand brought someone to mouth, release fingers
		"GTSvore_swallow",          // [6] Actor was swallowed by Giantess
		"GTSvore_swallow_sound",    // [7] Play gulp sound, eat actor completely (kill)
	};

	const std::vector<std::string_view> Anim_ThighSandwich = {
		"GTSsandwich_crouchstart",  // [0] Start air rumble and camera shake
		"GTSsandwich_grabactor",    // [1] Grab actor
		"GTSsandwich_crouchend",    // [2] Return to sit position
		"GTSsandwich_putactor",     // [3] Put actor on leg
		"GTSsandwich_enterloop",    // [4] Start idle loop with someone between legs
		"GTSsandwich_sandwichstart",// [5] Sandwich someone between legs, dealing damage and playing sound
		"GTSsandwich_sandwichhit",  // [6] Apply damage and sound
		"GTSsandwich_sandwichend",  // [7] Leg returns to idle position
		"GTSsandwich_exit",         // [8] Exit sandwich loop and leave animation in general
	};

	const std::vector<std::string_view> LegRumbleNodes = { // used with Anim_ThighCrush
		"NPC L Foot [Lft ]",
		"NPC R Foot [Rft ]",
		"NPC L Toe0 [LToe]",
		"NPC R Toe0 [RToe]",
		"NPC L Calf [LClf]",
		"NPC R Calf [RClf]",
		"NPC L FrontThigh",
		"NPC R FrontThigh",
		"NPC R RearCalf [RrClf]",
		"NPC L RearCalf [RrClf]",
	};
}


namespace Gts {
  AnimationEventData::AnimationEventData(const Actor& giant, const TESObjectREFR* tiny) : giant(giant), tiny(tiny) {}
  AnimationEvent::AnimationEvent(std::function<void(AnimationEventData&) callback, std::string group) : callback(callback), group(group) {}
  TriggerData::TriggerData(std::string_view behavor, std::string_view group) : behavor(behavor), group(group) {}

	AnimationManager& AnimationManager::GetSingleton() noexcept {
		static AnimationManager instance;
		return instance;
	}

	std::string AnimationManager::DebugName() {
		return "AnimationManager";
	}

  void AnimationManager::DataReady() {
    AnimationStomp::RegisterEvents();
    AnimationStomp::RegisterTriggers();

    AnimationCompat::RegisterEvents();
    AnimationCompat::RegisterTriggers();
  }

	void AnimationManager::Update() {
		auto player = PlayerCharacter::GetSingleton();
    if (player) {
      // Update fall behavor of player
      auto charCont = player->GetCharController();
      if (charCont) {
        player->SetGraphVariableFloat("GiantessVelocity", (charCont->outVelocity.quad.m128_f32[2] * 100)/get_visual_scale(player));
      }
    }
	}

	void AnimationManager::AdjustAnimSpeed(float bonus) {
		auto player = PlayerCharacter::GetSingleton();
    try {
      for (auto &[tag, data]: AnimationManager::GetSingleton().data.at(player).tags) {
        if (data.canEditAnimSpeed) {
          data.animSpeed += bonus;
          data.animSpeed = std::clamp(data.animSpeed, 0.0, 1.5);
        }
      }
    } catch (std::out_of_bounds e) {}
	}

  void AnimationManager::RegisterEvent(std::string_view name, std::string_view group, std::function<void(const AnimationEventData&)> func) {
    this->eventCallbacks.insert_or_assign(name, func, group);
  }

  void AnimationManager::RegisterTrigger(std::string_view trigger, std::string_view group, std::string_view behavior) {
    this->triggers.insert_or_assign(trigger, behavior, group);
  }


  void AnimationManager::StartAnim(std::string_view trigger, const Actor& giant) {
    this->StartAnim(trigger, giant, nullptr);
  }

  void AnimationManager::StartAnim(std::string_view trigger, const Actor& giant, const TESObjectREFR* tiny) {
    try {
      // Find the behavior for this trigger exit on catch if not
      auto& behavorToPlay = this->triggers.at(tag);
      auto& group = behavorToPlay.group;
      // Try to create anim data for actor
      this->data.try_emplace(giant);
      auto& actorData this->data[giant]; // Must exists now
      // Create the anim data for this group if not present
      try {
        auto& data = actorData.at(group);
      } catch (std::out_of_bounds) {
        log::warn("Animation {} already playing", trigger);
        return;
      }
      actorData.try_emplace(group, giant, tiny);
      // Run the anim
      giant->NotifyAnimationGraph(behavorToPlay.behavor);
    } catch (std::out_of_bounds) {
      log::error("Requested play of unknown animation named: {}", trigger);
      return;
    }
  }

	void AnimationManager::ActorAnimEvent(Actor* actor, const std::string_view& tag, const std::string_view& payload) {
    try {
      // Try to get the registerd anim for this tag
      auto& animToPlay = this->eventCallbacks.at(tag);
      // If data dosent exist then insert with default
      this->data.try_emplace(actor);
      auto& actorData = this->data[actor];
      let group animToPlay.group;
      // If data dosent exist this will insert it with default
      actorData.try_emplace(group, actor, nullptr);
      // Get the data or the newly inserted data
      auto& data = actorData[group];
      // Call the anims function
      animToPlay.callback(data);
      // If the stage is 0 after an anim has been played then
      //   delete this data so that we can reset for the next anim
      if (data.stage == 0) {
        actorData.erase(group);
      }

    } catch (std::out_of_bounds e) {}
	}
}
