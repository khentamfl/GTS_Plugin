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
  bool AttachTo(T& anyGiant, U& anyTiny, NiPoint3 point) {
    Actor* giant =  GetActorPtr(anyGiant);
    Actor* tiny =  GetActorPtr(anyTiny);

    if (!giant) {
      return false;
    }
    if (!tiny) {
      return false;
    }



    tiny->SetPosition(point, true);

    auto charcont = tiny->GetCharController();
    if (charcont) {
      charcont->SetLinearVelocityImpl((0.0, 0.0, 0.0, 0.0)); // Needed so Actors won't fall down.
    }

    return true;
  }
  template<typename T, typename U>
  bool AttachTo(T& anyGiant, U& anyTiny, std::string_view bone_name) {
    Actor* giant =  GetActorPtr(anyGiant);
    if (!giant) {
      return false;
    }
    auto bone = find_node(giant, bone_name);
    if (!bone) {
      return false;
    }
    return AttachTo(anyGiant, anyTiny, bone->world.translate);
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
  bool AttachToHand(T& anyGiant, U& anyTiny) {
    return AttachTo(anyGiant, anyTiny, "NPC L Finger02 [LF02]");
  }

  template<typename T, typename U>
  bool AttachToClevage(T& anyGiant, U& anyTiny) {
    Actor* giant =  GetActorPtr(anyGiant);
    if (!giant) {
      return false;
    }

    auto breastLNode = find_node(giant, "L Breast03")->world.translate;
    if (!breastLNode) {
      return false;
    }
    auto breastL = breastL->world.translate;

    auto breastRNode = find_node(giant, "R Breast03")->world.translate;
    if (!breastRNode) {
      return false;
    }
    auto breastR = breastR->world.translate;

    auto spine2Node = find_node(giant, "NPC Spine2 [Spn2]")->world.translate;
    if (!spine2Node) {
      return false;
    }
    auto spine2 = spine2Node->world.translate;

    float forwardAmount = 1.2;

    auto breastForward = ((breastL - spine2) + (breastR - spine2))  * forwardAmount / 2 + spine2;
    return AttachTo(anyGiant, anyTiny, breastForward);
  }
}
