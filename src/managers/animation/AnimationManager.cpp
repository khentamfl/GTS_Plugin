#include "managers/animation/AnimationManager.hpp"
#include "managers/animation/ThighCrush.hpp"
#include "managers/animation/ActorVore.hpp"
#include "managers/animation/Compat.hpp"
#include "managers/animation/Stomp.hpp"
#include "data/persistent.hpp"
#include "scale/scale.hpp"

using namespace RE;
using namespace Gts;
using namespace std;

namespace {
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

}


namespace Gts {
	AnimationEventData::AnimationEventData(Actor& giant, TESObjectREFR* tiny) : giant(giant), tiny(tiny) {
	}
	AnimationEvent::AnimationEvent(std::function<void(AnimationEventData&)> a_callback,  std::string a_group) : callback(a_callback), group(a_group) {
	}
	TriggerData::TriggerData( std::vector< std::string_view> behavors,  std::string_view group) : behavors({}), group(group) {
		for (auto& sv: behavors) {
			this->behavors.push_back(std::string(sv));
		}
	}

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

		AnimationThighCrush::RegisterEvents();
		AnimationThighCrush::RegisterTriggers();

		AnimationCompat::RegisterEvents();
		AnimationCompat::RegisterTriggers();

		AnimationActorVore::RegisterEvents();
		AnimationActorVore::RegisterTriggers();
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

	void AnimationManager::Reset() {
		this->data.clear();
	}
	void AnimationManager::ResetActor(Actor* actor) {
		this->data.erase(actor);
	}

	void AnimationManager::AdjustAnimSpeed(float bonus) {
		auto player = PlayerCharacter::GetSingleton();
		try {
			for (auto& [tag, data]: AnimationManager::GetSingleton().data.at(player)) {
				if (data.canEditAnimSpeed) {
					data.animSpeed += bonus;
					ConsoleLog::GetSingleton()->Print("Data: Anim Speed of %s is %g", player->GetDisplayFullName(), data.animSpeed);
				}
				//data.animSpeed = std::clamp(data.animSpeed, 0.0f, 1.5f);
			}
		} catch (std::out_of_range e) {}
	}

  float AnimationManager::GetAnimSpeed(Actor* actor) {
    float speed = 1.0;
    if (actor) {
      auto saved_data = Gts::Persistent::GetSingleton().GetData(actor);
      if (saved_data) {
        if (saved_data->anim_speed > 0.0) {
          speed *= saved_data->anim_speed;
        }
      }

      try {
        float totalSpeed = 1.0;
  			for (auto& [tag, data]: AnimationManager::GetSingleton().data.at(actor)) {
  					totalSpeed *= data.animSpeed;
  			}
        speed *= totalSpeed;
  		} catch (std::out_of_range e) {
      }
    }
    return speed;
  }

	void AnimationManager::RegisterEvent( std::string_view name,  std::string_view group, std::function<void(AnimationEventData&)> func) {
		AnimationManager::GetSingleton().eventCallbacks.try_emplace(std::string(name), func, std::string(group));
		log::info("Registering Event: Name {}, Group {}", name, group);
	}

	void AnimationManager::RegisterTrigger( std::string_view trigger,  std::string_view group,  std::string_view behavior) {
		AnimationManager::RegisterTriggerWithStages(trigger, group, {behavior});
		log::info("Registering Trigger: {}, Group {}, Behavior {}", trigger, group, behavior);
	}

	void AnimationManager::RegisterTriggerWithStages( std::string_view trigger,  std::string_view group,  std::vector< std::string_view> behaviors) {
		if (behaviors.size() > 0) {
			AnimationManager::GetSingleton().triggers.try_emplace(std::string(trigger), behaviors, group);
			log::info("Registering Trigger With Stages: {}, Group {}", trigger, group);
		}
	}


	void AnimationManager::StartAnim( std::string_view trigger, Actor& giant) {
		AnimationManager::StartAnim(trigger, giant, nullptr);

	}
	void AnimationManager::StartAnim( std::string_view trigger, Actor* giant) {
		if (giant) {
			AnimationManager::StartAnim(trigger, *giant);
			log::info("Starting Trigger {} for {}", trigger, giant->GetDisplayFullName());
		}
	}

	void AnimationManager::StartAnim( std::string_view trigger, Actor& giant, TESObjectREFR* tiny) {
		try {
			auto& me = AnimationManager::GetSingleton();
			// Find the behavior for this trigger exit on catch if not
			auto& behavorToPlay = me.triggers.at(std::string(trigger));
			auto& group = behavorToPlay.group;
			// Try to create anim data for actor
			me.data.try_emplace(&giant);
			auto& actorData = me.data.at(&giant); // Must exists now
			// Create the anim data for this group if not present
			actorData.try_emplace(group, giant, tiny);
			// Run the anim
			//log::info("Playing Trigger {} for {}", trigger, giant.GetDisplayFullName());
			giant.NotifyAnimationGraph(behavorToPlay.behavors[0]);
		} catch (std::out_of_range) {
			log::error("Requested play of unknown animation named: {}", trigger);
			return;
		}
	}
	void AnimationManager::StartAnim(std::string_view trigger, Actor* giant, TESObjectREFR* tiny) {
		if (giant) {
			AnimationManager::StartAnim(trigger, *giant, tiny);
		}
	}

	void AnimationManager::NextAnim(std::string_view trigger, Actor& giant) {
		try {
			auto& me = AnimationManager::GetSingleton();
			// Find the behavior for this trigger exit on catch if not
			auto& behavorToPlay = me.triggers.at(std::string(trigger));
			auto& group = behavorToPlay.group;
			// Get the actor data OR exit on catch
			auto& actorData = me.data.at(&giant);
			// Get the event data of exit on catch
			auto& eventData = actorData.at(group);
			std::size_t currentTrigger = eventData.currentTrigger;
			// Run the anim
			if (behavorToPlay.behavors.size() < currentTrigger) {
				giant.NotifyAnimationGraph(behavorToPlay.behavors[currentTrigger]);
			}
		} catch (std::out_of_range) {
			return;
		}
	}
	void AnimationManager::NextAnim(std::string_view trigger, Actor* giant) {
		if (giant) {
			AnimationManager::NextAnim(trigger, *giant);
		}
	}

	void AnimationManager::ActorAnimEvent(Actor* actor, const std::string_view& tag, const std::string_view& payload) {
		try {
			if (actor) {
				// Try to get the registerd anim for this tag
				auto& animToPlay = this->eventCallbacks.at(std::string(tag));
				// If data dosent exist then insert with default
				this->data.try_emplace(actor);
				auto& actorData = this->data.at(actor);
				auto group = animToPlay.group;
				// If data dosent exist this will insert it with default
				actorData.try_emplace(group, *actor, nullptr);
				// Get the data or the newly inserted data
				auto& data = actorData.at(group);
				// Call the anims function
				animToPlay.callback(data);
				// If the stage is 0 after an anim has been played then
				//   delete this data so that we can reset for the next anim
				if (data.stage == 0) {
					actorData.erase(group);
				}
			}
		} catch (std::out_of_range e) {}
	}

	// Get the current stage of an animation group
	std::size_t AnimationManager::GetStage(Actor& actor,  std::string_view group) {
		try {
			auto& me = AnimationManager::GetSingleton();
			return me.data.at(&actor).at(std::string(group)).stage;
		} catch (std::out_of_range e) {
			return 0;
		}
	}
	std::size_t AnimationManager::GetStage(Actor* actor,  std::string_view group) {
		if (actor) {
			return AnimationManager::GetStage(*actor, group);
		} else {
			return 0;
		}
	}

	// Check if any currently playing anim disabled the HHs
	bool AnimationManager::HHDisabled(Actor& actor) {
		try {
			auto& me = AnimationManager::GetSingleton();
			auto& actorData = me.data.at(&actor);
			for ( auto &[group, data]: actorData) {
				if (data.disableHH) {
					return true;
				}
			}
			return false;
		} catch (std::out_of_range e) {
			return false;
		}
	}
	bool AnimationManager::HHDisabled(Actor* actor) {
		if (actor) {
			return AnimationManager::HHDisabled(*actor);
		} else {
			return false;
		}
	}
}
