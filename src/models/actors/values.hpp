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

struct ActorValueMod {
  std::string name;
  float amount;
  double startTime;
  float duration;
};

class GActorValue {
  public:
    float Get() {
      return this->value.value;
    }

    // Apply changes from temporary effects
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

      float maxValue = this->baseMax;
      for (const auto& [key, mod]: this->maxModifiers) {
        if (fabs(mod.duration) < 1e-4 || mod.startTime + mod.duration <= currentTime) {
          maxValue += mod.amount;
        } else {
          this->modifiers.erase(key);
        }
      }
      this->maxValue.target = maxValue;
    }
  private:
    std::string name;
    float base;
    Spring value;
    float baseMax;
    Spring maxValue;
    std::uint32_t modEntry = 0;

    std::unordered_set<std::string, std::uint32_t> modifierMap;
    std::unordered_set<std::uint32_t, ActorValueMod> modifiers;

    std::unordered_set<std::string, std::uint32_t> maxModifierMap;
    std::unordered_set<std::uint32_t, ActorValueMod> maxModifiers;
};
