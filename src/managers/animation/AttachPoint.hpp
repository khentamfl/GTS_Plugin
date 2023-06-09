#pragma once
#include "node.hpp"

using namespace RE;

namespace Gts {

  Actor* GetActorPtr(Actor* actor) {
    return actor;
  }

  Actor* GetActorPtr(Actor& actor) {
    return &actor;
  }

  Actor* GetActorPtr(ActorHandle& actor) {
    if (!actor) {
      return nullptr;
    }
    return actor.get().get();
  }
  Actor* GetActorPtr(FormID formId) {
    Actor* actor = TESForm::LookupByID<Actor>(formId);
    if (!actor) {
      return nullptr;
    }
    return actor;
  }

  template<typename T, typename U>
  bool AttachTo(T& anyGiant, U& anyTiny, std::string_view bone_name) {
      Actor* giant =  GetActorPtr(anyGiant);
      Actor* tiny =  GetActorPtr(anyTiny);

      if (!giant) {
  			return false;
  		}
  		if (!tiny) {
  			return false;
  		}

      auto bone = find_node(giant, bone_name);
      if (!bone) {
				return false;
			}

			tiny->SetPosition(bone->world.translate, true);

			auto charcont = tiny->GetCharController();
			if (charcont) {
				charcont->SetLinearVelocityImpl((0.0, 0.0, 0.0, 0.0)); // Needed so Actors won't fall down.
			}

      return true;
  }

  template<typename T, typename U>
  bool AttachToObjectA(T& anyGiant, U& anyTiny) {
    return AttachTo(anyGiant, anyTiny, "AnimObjectA");
  }

  template<typename T, typename U>
  bool AttachToObjectB(T& anyGiant, U& anyTiny) {
    return AttachTo(anyGiant, anyTiny, "AnimObjectB");
  }

  template<typename T, typename U>
  bool AttachToObjectHand(T& anyGiant, U& anyTiny) {
    return AttachTo(anyGiant, anyTiny, "NPC L Finger02 [LF02]");
  }
}
