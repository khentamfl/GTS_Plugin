module;

export module Giant;

export namespace Gts {
  class Giant;
  using Tiny = Giant; // All giants are also tiny to someone all tinys can be giant to another

  class Giant {
    public:
      // == Allow move
      Giant(Giant&&) = default;
      // == No copy ==
			Giant(Giant const&) = delete;
			Giant& operator=(Giant const&) = delete;


      // ================
      // == Basic Info ==
      // ================
      std::string Name() {
        return Get()->GetDisplayFullName();
      }

      // ===========
      // == Scale ==
      // ===========

      // This is the total scale after all effects
      //   this includes effects like race menu scale
      //
      // To save computation the nodes scale expect our
      // node is cached from start of the frame
      float Scale() {
        return 1.0;
      }

      // This is the scale our mod adds
      float GiantScale() {
        return 1.0;
      }

      // This is the total scale after all effects
      //   this includes effects like race menu scale
      //
      // This one is computed relative to a standard actor height
      //   so a skyrim giant would start off as having extra scale
      //   to steal
      float Height() {
        return 1.0;
      }


      // Grow by an amount, unbuffed this is a mod scale
      //
      // This function incorperates buffs and any calls and check
      //
      // This function is the PREFFERED grow method
      void Grow(const float& amt) {

      }

      // Shrink by an amount, unbuffed this is a mod scale
      //
      // This function incorperates buffs and any calls and check
      //
      // This function is the PREFFERED shrink method
      void Shrink(const float& amt) {

      }

      // Steal size from another actor
      //
      // This function incoperates buffs/resistances
      //
      // it is based on the Height of the Tiny so that
      // big animals (like mamoths have more size to give, but take longer)
      void Steal(Tiny& tiny, const float& amt) {

      }



      // Creation
      Giant* FromActorFormID(FormID formID) {
        auto* actor = TESForm::LookupByID<Actor>(formID);
        return FromActorPtr(actor);
      }
      Giant* FromActorPtr(Actor* actor) {
        if (!actor) {
          return nullptr;
        }
        auto& giant = FromActor(*actor);

        return &giant;
      }
      Giant& FromActor(Actor& actor) {
        auto key = actor.formID;
        static std::unordered_map<FormID, Giant> all_giants;
        try {
          auto& nobind = all_giants.at(key);
        } catch (std::out_of_range e) {
          Giant giant = Giant(&actor);
          all_giants.try_emplace(key, std::move(giant));
        }

        return all_giants.at(key);
      }

      // ===========
      // == Defer ==
      // ===========

      // Get the Actor*
      Actor* Get() {
        return actor.get().get();
      }

      // Automatic get the Actor*
      Actor* operator->() {
        return Get();
      }
      Actor& operator*() {
        return *Get();
      }

    protected:
      Giant(Actor* giant): actor(giant->CreateRefHandle()) {
        // Create new
      }

      // Data
      ActorHandle actor;
  };
}
