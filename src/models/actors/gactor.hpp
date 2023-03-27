// Represents a GTSActor
// Holds a reference and convience fuctions as well as data for an actor
//
// This is the generic one for Any kind of actor
//  You should cast to NPC/Creature/Player varients as needed
//

#include "models/serde.hpp"
#include "models/actors.base"
#include "models/actors/values.hpp"
#include "models/actors/flags.hpp"

using namespace RE;

namespace {
  const float DEFAULT_MAX_SCALE = 65535.0;
	const float DEFAULT_HALF_LIFE = 1.0;
  const float DEFAULT_SCALE_MULTI = 1.0;
}

namespace Gts {
  class GActor: public GActorBase, public AvHolder, public FlagHolder {
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
      this->CreateAv("scale", GActorValue("scale", 1.0, DEFAULT_MAX_SCALE, DEFAULT_SCALE_MULTI, DEFAULT_HALF_LIFE));
      // Animation speed
      this->CreateAv("animSpeed", GActorValue("animSpeed", 1.0));
      // Bonus HP
      this->CreateAv("bonusHp", GActorValue("bonusHp", 0.0));
      // Bonus Carry
      this->CreateAv("bonusCarry", GActorValue("bonusCarry", 0.0));
      // Bonus Damage Multi
      this->CreateAv("damageMulti", GActorValue("damageMulti", 1.0));
      // Bonus Sprint Damage Multi
      this->CreateAv("sprintDamageMulti", GActorValue("sprintDamageMulti", 1.0));
      // Bonus Fall Damage Multi
      this->CreateAv("fallDamageMulti", GActorValue("fallDamageMulti", 1.0));
      // TODO: Move to NPC
      // Bonus HH Damage Multi
      this->CreateAv("hhDamageMulti", GActorValue("hhDamageMulti", 1.0));
      // Weakness to Size
      this->CreateAv("weaknessSize", GActorValue("weaknessSize", 1.0));
      // Size Reserve used to store and realease size at a different time
      this->CreateAv("sizeReserve", GActorValue("sizeReserve", 0.0));
    }

    // The Form ID
    virtual FormID FormID() {
      return this->actor->formId;
    }

    // The name of the actor
    virtual std::string Name() {
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
        TESForm* actor = TESForm::LookupByID<Actor>(formID);
        if (actor) {
          this->actor = actor.CreateRefHandle();
          AvHolder::from_toml(toml::find(v, "avs"));
          FlagHolder::from_toml(toml::find(v, "flags"));
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
            {"avs", AvHolder::into_toml()},
            {"flags", FlagHolder::into_toml()},
          }
        );
    }

  protected:
    ActorHandle actor;
  };
}
