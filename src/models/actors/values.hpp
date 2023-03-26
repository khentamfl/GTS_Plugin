// Contains genric actor value
//
// Our AVs will have a
// value, max and a smoothing effect for chanegs
//
// Changes can be applied with temporary or permentant modifiers
//
// These modifies can be named so that for example applying a temp
// modifier with name GrowthBonusPostion twice will not stack but will reset the temp time
//

#include "spring.hpp"
#include "data/time.hpp"
#include <atomic>

struct ActorValueMod {
  std::string name;
  float amount;
  double startTime;
  float duration;

  ActorValueMod(std::string_view name, float amount, float duration) : name(std::string(name)), amount(amount), startTime(TIme::WorldTimeElapsed), duration(duration) {}
};

struct ActorValueModSet {
  float base = 0.0;
  Spring value = Spring(0.0, 1.0);
  atomic_int32_t unique_ident;
  std::unordered_set<std::string, ActorValueMod> modifiers;

  ActorValueModSet(float base, float halflife) : base(base), Spring(Spring(base, halflife)) {}

  void Update() {
    float value = this->base;
    double currentTime = Time::WorldTimeElapsed();
    for (const auto& [key, mod]: this->modifiers) {
      if (fabs(mod.duration) < 1e-4 || mod.startTime + mod.duration <= currentTime) {
        value += mod.amount;
      } else {
        this->modifiers.erase(key);
      }
    }
    this->value.target = value;
  }

  float Get() {
    return this->value.value;
  }

  float GetT() {
    return this->value.target;
  }

  void Set(std::string_view name, float amount, float duration) {
    this->Mod(name, amount - this->value.target, duration);
  }

  void Mod(std::string_view namesv, float amount, float duration) {
    if (fabs(amount) > 1e-3) {
      if (duration < 1e-3 && namesv == "") {
        // If permenant AND has no name we can just mod the base value
        base += amount;
      } else {
        std::string name = "";
        if (namesv == "") {
          // No name but duration is non zero so we need to track it
          name = std::format("UNNAMED_{}", this->unique_ident.fetch_add(1, std::memory_order_relaxed);)
        } else {
          name = std::string(namesv);
        }
        this->modifiers.try_emplace(name, name, amount, duration);
        auto& reapply = this->modifiers.at(std::string(name));
        // In case we didn't make a new we reset the values
        reapply.amount = amount;
        reapply.duration = duration;
        reapply.startTime = Time::WorldTimeElapsed();
      }
    }
  }
};

class GActorValue {
  public:
    // Get total value as
    // (base + perm + temp) with smoothing
    // not exceeding maximum
    // then multiply but the mutlpier
    float Get() {
      return std::min(this->value.Get(), this->max.Get()) * this->.multi.Get();
    }

    // Same Get but wothout smoothing
    float GetT() {
      return std::min(this->value.GetT(), this->max.GetT()) * this->.multi.GetT();
    }

    // Get the smoothed max value
    float GetMax() {
      return this->max.Get();
    }

    // Get the unsmoothed max value
    float GetMaxT() {
      return this->max.GetT();
    }

    // Get the smoothed multiplier value
    float GetMulti() {
      return this->multi.Get();
    }

    // Get the unsmoothed multiplier value
    float GetMultiT() {
      return this->multi.GetT();
    }

    // Set the value to something specific
    // This will look at the the current (unsmoothed value)
    // and apply a modifier based on that (if has duration)
    // or change it directly if not
    //
    // It is usually better to use mod
    void Set(std::string_view name, float value, float duration) {
      this->value.Set(name, value, duration);
    }
    void Set(std::string_view name, float value) {
      this->value.Set(name, value, 0.0);
    }
    void Set(float value, float duration) {
      this->value.Set("", name, value, duration);
    }
    void Set(float value) {
      this->value.Set("", name, value, 0.0);
    }

    // Same as set but for max value
    void SetMax(std::string_view name, float value, float duration) {
      this->max.Set(name, value, duration);
    }
    void SetMax(std::string_view name, float value) {
      this->max.Set(name, value, 0.0);
    }
    void SetMax(float value, float duration) {
      this->max.Set("", name, value, duration);
    }
    void SetMax(float value) {
      this->max.Set("", name, value, 0.0);
    }

    // Same as set but for mutliplier value
    void SetMulti(std::string_view name, float value, float duration) {
      this->multi.Set(name, value, duration);
    }
    void SetMulti(std::string_view name, float value) {
      this->multi.Set(name, value, 0.0);
    }
    void SetMulti(float value, float duration) {
      this->multi.Set("", name, value, duration);
    }
    void SetMulti(float value) {
      this->multi.Set("", name, value, 0.0);
    }

    // Mod the value by an amount
    // This will look at the the current (unsmoothed value)
    // and apply a modifier based on that (if has duration)
    // or change it directly if not
    void Mod(std::string_view name, float value, float duration) {
      this->value.Mod(name, value, duration);
    }
    void Mod(std::string_view name, float value) {
      this->value.Mod(name, value, 0.0);
    }
    void Mod(float value, float duration) {
      this->value.Mod("", name, value, duration);
    }
    void Mod(float value) {
      this->value.Mod("", name, value, 0.0);
    }

    // Same as set but for max value
    void ModMax(std::string_view name, float value, float duration) {
      this->max.Mod(name, value, duration);
    }
    void ModMax(std::string_view name, float value) {
      this->max.Mod(name, value, 0.0);
    }
    void ModMax(float value, float duration) {
      this->max.Mod("", name, value, duration);
    }
    void ModMax(float value) {
      this->max.Mod("", name, value, 0.0);
    }

    // Same as set but for mutliplier value
    void ModMulti(std::string_view name, float value, float duration) {
      this->multi.Mod(name, value, duration);
    }
    void ModMulti(std::string_view name, float value) {
      this->multi.Mod(name, value, 0.0);
    }
    void ModMulti(float value, float duration) {
      this->multi.Mod("", name, value, duration);
    }
    void ModMulti(float value) {
      this->multi.Mod("", name, value, 0.0);
    }

    // Apply changes from temporary effects
    void Update() {
      this->value.Update();
      this->max.Update();
      this->multi.Update();
    }
  private:

    std::string name;
    // Value
    ActorValueModSet value;
    ActorValueModSet max;
    ActorValueModSet multi;
};
