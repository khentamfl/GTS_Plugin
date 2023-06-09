#pragma once

using namespace RE;

namespace Gts {

  void GetActor(Actor& actor) -> Actor& {
    return actor;
  }
  void GetActor(Actor* actor) -> Actor& {
    if (!actor) {
      throw std::runtime_error("Pointer to actor is invalid");
    }
    return GetActor(*actor);
  }
  void GetActor(ActorHandle& actor) -> Actor& {
    if (!actor) {
      throw std::runtime_error("ActorHandle is invalid");
    }
    return GetActor(*actor.get().get());
  }
  void GetActor(FormID formId) -> Actor& {
    Actor* actor = TESForm::LookupByID<Actor>(formId);
    if (!actor) {
      throw std::runtime_error("FormID is not for an actor");
    }
    return GetActor(*actor.get().get());
  }

  template<T, U>
  void AttachToObjectA(T maybeGiant, U maybeTiny) {
      Actor& giant;
      Actor& tiny;
      try {
         giant = GetActor(maybeGiant);
         tiny = GetActor(maybeTiny);
      } catch (const std::runtime_error& e) {
        return;
      }
      // Ready
  }
}
