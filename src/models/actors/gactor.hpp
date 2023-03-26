// Represents a GTSActor
// Holds a reference and convience fuctions as well as data for that actor
//
// It's parent classes should contain the needed modules such as crush/vore/grow
//
// This GActor itself should be subclassed for specific actors such as Player
// in models/personas this will allow for override of abilities for certain actors
//

#include "models/serde.hpp"
#include "models/actors/values.hpp"

using namespace RE;

namespace {
  const float DEFAULT_MAX_SCALE = 65535.0;
	const float DEFAULT_HALF_LIFE = 1.0;
  const float DEFAULT_SCALE_MULTI = 1.0;
}

namespace Gts {
  class GActor {
  public:
    // Default constructor:
    //  Needs to be included so that we can convert from toml
    virtual GActor(): actor(nullptr) {

    }

    virtual GActor(Actor* actor): actor(actor ? actor->CreateRefHandle() : nullptr) {
      if (!actor) {
        return; // Just give up with the handle pointing to nullptr
      }
      // Scale Actor Value
      //
      // Default is 1.0
      this->gactorValues.try_emplace("scale", GActorValue("scale", 1.0, DEFAULT_MAX_SCALE, DEFAULT_SCALE_MULTI, DEFAULT_HALF_LIFE));
    }

    // The Form ID
    virtual FormID GetFormID() {
      return this->actor->formId;
    }

    // The name of the actor
    virtual std::string GetName() {
      return this->actor->GetDisplayNameFull();
    }

    // The type of Actor Class
    //
    // This is crucial for correct save/load
    // as it determines how it is saved/loaded from
    // the cosave
    virtual std::string GetCls() {
      return "Base";
    }

    // Used for the toml coversions
    virtual void from_toml(const toml::value& v)
    {
      // Try to get the actor from the formID
      RE::FormID formID = toml::find<RE::FormID>(v, "formid", 0u32);

      if (formID > 0) {
        TESForm* actor_form = TESForm::LookupByID<Actor>(formID);
        if (actor_form) {
          Actor* actor = skyrim_cast<Actor*>(actor_form);
          if (actor) {
            this->actor = actor.CreateRefHandle();
            this->gactorValues = toml::find<std::unordered_map<std::string, GActorValue>>(v, "avs");
          } else {
            log::warn("Actor ID {:X} could not be found after loading the toml.", formID);
          }
        } else {
          log::warn("Actor ID {:X} could not be found after loading the toml.", formID);
        }
      }
    }
    virtual toml::value into_toml() const // you need to mark it const.
    {
        return toml::value(
          {
            {"formid", this->actor->formId},
            {"avs", this->gactorValues},
          }
        );
    }

    // New Methods


  protected:
    ActorHandle actor;

  private:
    std::unordered_map<std::string, GActorValue> gactorValues;
  };
}
