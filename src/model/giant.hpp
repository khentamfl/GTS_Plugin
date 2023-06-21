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

      // Basic Info
      std::string Name();

      float Scale();


      // Represetation as other types
      Actor* Get();


      // Creation
      Giant* FromActorFormID(FormID formID);
      Giant* FromActorPtr(Actor* actor);
      Giant& FromActor(Actor& actor);

    protected:
      Giant(Actor* giant);

      // Data
      ActorHandle actor;
  };

  using Tiny = Giant; // All giants are also tiny to someone all tinys can be giant to another
}
