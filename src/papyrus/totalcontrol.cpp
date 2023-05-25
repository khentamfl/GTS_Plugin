#include "papyrus/scale.hpp"
#include "scale/scale.hpp"
#include "scale/modscale.hpp"
#include "data/persistent.hpp"
#include "managers/GtsManager.hpp"
#include "managers/Rumble.hpp"
#include "data/runtime.hpp"


using namespace SKSE;
using namespace Gts;
using namespace RE;
using namespace RE::BSScript;

namespace {
	constexpr std::string_view PapyrusClass = "GtsControl";

	void GrowTeammate(StaticFunctionTag*, float power) {
    auto casterRef = PlayerCharacter::GetSingleton();
    if (!casterRef) {
      return;
    }
    for (auto targetRef: FindTeammates()) {
      if (!targetRef) {
        continue;
      }
      float Volume = clamp(0.50, 1.0, get_visual_scale(targetRef));
      Runtime::PlaySound("growthSound", targetRef, Volume, 0.0);

      // Thread safe handles
      ActorHandle casterHandle = casterRef->CreateRefHandle();
      ActorHandle targetHandle = targetRef->CreateRefHandle();

      const float DURATION = 2.0;

      TaskManager::RunFor(DURATION, [=](auto& progressData){
        if (!casterHandle) {
          return false;
        }
        if (!targetHandle) {
          return false;
        }
        float timeDelta = progressData.delta;

        auto target = targetHandle.get().get();
        auto caster = casterHandle.get().get();

        float target_scale = get_visual_scale(target);
        float magicka = clamp(0.05, 1.0, GetMagikaPercentage(caster));

        float bonus = 1.0;
        if (Runtime::HasMagicEffect(caster, "EffectSizeAmplifyPotion")) {
          bonus = target_scale * 0.25 + 0.75;
        }

        if (target_scale > get_natural_scale(target)) {
          DamageAV(caster, ActorValue::kMagicka, 0.45 * (target_scale * 0.25 + 0.75) * magicka * bonus * timeDelta * power);
          mod_target_scale(target, 0.0030 * magicka * bonus * timeDelta * power);
          Rumble::Once("GrowOtherButton", target, 1.0, 0.05);
        }

        return true;
      });

    }
  }

  void ShrinkTeammate(StaticFunctionTag*, float power) {
    auto casterRef = PlayerCharacter::GetSingleton();
    if (!casterRef) {
      return;
    }
    for (auto targetRef: FindTeammates()) {
      if (!targetRef) {
        continue;
      }
      float Volume = clamp(0.50, 1.0, get_visual_scale(targetRef));
  		Runtime::PlaySound("shrinkSound", targetRef, Volume, 0.0);

      // Thread safe handles
      ActorHandle casterHandle = casterRef->CreateRefHandle();
      ActorHandle targetHandle = targetRef->CreateRefHandle();

      const float DURATION = 2.0;

      TaskManager::RunFor(DURATION, [&](auto& progressData){
        if (!casterHandle) {
          return false;
        }
        if (!targetHandle) {
          return false;
        }
        float timeDelta = progressData.delta;

        auto target = targetHandle.get().get();
        auto caster = casterHandle.get().get();

        float target_scale = get_visual_scale(target);
    		float magicka = clamp(0.05, 1.0, GetMagikaPercentage(caster));

    		float bonus = 1.0;
    		if (Runtime::HasMagicEffect(caster, "EffectSizeAmplifyPotion")) {
    			bonus = target_scale * 0.25 + 0.75;
    		}

    		if (target_scale > get_natural_scale(target)) {
    			DamageAV(caster, ActorValue::kMagicka, 0.25 * (target_scale * 0.25 + 0.75) * magicka * bonus * timeDelta * power);
    			mod_target_scale(target, -0.0030 * magicka * bonus * timeDelta * power);
    			Rumble::Once("ShrinkOtherButton", target, 1.0, 0.05);
    		}
        return true;
      });

    }
  }
}

namespace Gts {
	bool register_total_control(IVirtualMachine* vm) {
		vm->RegisterFunction("GrowTeammate", PapyrusClass, GrowTeammate);
    vm->RegisterFunction("ShrinkTeammate", PapyrusClass, ShrinkTeammate);

		return true;
	}
}
