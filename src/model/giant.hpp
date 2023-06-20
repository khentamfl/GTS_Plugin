#pragma once
// Wrapper around an Actor to handle giant stuff
//

using namespace RE;

namespace Gts {
  class Giant {
    public:
      // No copy
			Giant(Giant const&) = delete;
			Giant& operator=(Giant const&) = delete;

      std::string Name() {
        return Get()->GetDisplayFullName();
      }

      Actor* Get() {
        return actor.get().get();
      }


      Giant* FromActor(Actor* actor) {
        if (!actor) {
          return nullptr;
        }
        auto key = actor->formID;
        static std::unordered_map<FormID, Giant> giants;
        giants.try_emplace(key, actor);

        return &giants.at(key);
      }
    protected:
      Giant(Actor* giant): actor(giant->CreateRefHandle()) {
        // Create new
      }
      ActorHandle actor;
  };

  using Tiny = Giant; // All giants are also tiny to someone all tinys can be giant to another
}
