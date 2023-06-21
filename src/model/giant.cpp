#include "model/giant.hpp"
// Wrapper around an Actor to handle giant stuff
//

using namespace RE;

namespace Gts {
  std::string Giant::Name() {
    return Get()->GetDisplayFullName();
  }

  float Giant::Scale() {
    return get_visual_scale(Get());
  }

  Actor* Giant::Get() {
    return actor.get().get();
  }

  Giant* Giant::FromActorFormID(FormID formID) {
    auto* actor = TESForm::LookupByID<Actor>(formID);
    return FromActorPtr(actor);
  }
  Giant* Giant::FromActorPtr(Actor* actor) {
    if (!actor) {
      return nullptr;
    }
    auto& giant = FromActor(*actor);

    return &giants;
  }


  Giant& Giant::FromActor(Actor& actor) {
    auto key = actor->formID;
    static std::unordered_map<FormID, Giant> all_giants;
    all_giants.try_emplace(key, actor);

    return &all_giants.at(key);
  }

  Giant::Giant(Actor* giant): actor(giant->CreateRefHandle()) {
    // Create new
  }
}
