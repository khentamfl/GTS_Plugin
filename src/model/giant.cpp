#include "model/giant.hpp"
// Wrapper around an Actor to handle giant stuff
//

using namespace RE;

namespace Gts::Giant {
  std::string Name() {
    return Get()->GetDisplayFullName();
  }

  float Scale() {
    return get_visual_scale(Get());
  }

  Actor* Get() {
    return actor.get().get();
  }

  Giant* FromActorPtr(Actor* actor) {
    if (!actor) {
      return nullptr;
    }
    auto& giant = FromActor(*actor);

    return &giants;
  }


  Giant& FromActor(Actor& actor) {
    auto key = actor->formID;
    static std::unordered_map<FormID, Giant> all_giants;
    all_giants.try_emplace(key, actor);

    return &all_giants.at(key);
  }

  Giant(Actor* giant): actor(giant->CreateRefHandle()) {
    // Create new
  }
}
