#include "hooks/jump.hpp"
#include "hooks/callhook.hpp"
#include "scale/scale.hpp"

using namespace RE;
using namespace SKSE;

namespace {
  Actor* FindActor(bhkCharacterController* charCont) {
    for (auto actor: find_actors()) {
      if (charCont == actor->GetCharController()) {
        return actor;
      }
    }
    return nullptr;
  }
}

namespace Hooks {

	void Hook_Jumping::Hook(Trampoline& trampoline) {


		static FunctionHook<float(bhkCharacterController* a_this)> GetFallDistance(
			REL::RelocationID(76430, 78269),
			[](auto* a_this){
          float result = GetFallDistance(a_this);
          log::info("GetFallDistance: {}", result);
          auto actor = FindActor(a_this);
          if (actor) {
            float scale = get_visual_scale(actor);
            if (scale > 1e-4) {
              result /= scale;
              log::info("  - Changed to {} for {}", result, actor->GetDisplayFullName());
            }
          }

    			return result;
  			}
			);

      // AE 1402bc7c3
      // SE 1402aa40c
      //
      // Is used in the jump anim event handller
      //
      REL::Relocation<uintptr_t> hook{REL::RelocationID(41811, 42892)};
      _GetScaleJumpHook = trampoline.write_call<5>(hook.address() + RELOCATION_OFFSET(0x4d, 0x4d), GetScaleJumpHook);

      // static CallHook<float(TESObjectREFR*)> GetScaleHookJumpAnimEvent(RELOCATION_ID(41811, 42892),  0x4d,
      // [](auto* self) {
      //     float result = GetScaleHookJumpAnimEvent(self);
      //     Actor* actor = skyrim_cast<Actor*>(self);
      //     if (actor) {
      //       float scale = get_visual_scale(actor);
      //       if (scale > 1e-4) {
      //         log::info("Jump Hook: {} for {}", scale, actor->GetDisplayFullName());
      //         result *= scale;
      //       }
      //     }
      //     return result;
      // });

	}

  float Hook_Jumping::GetScaleJumpHook(TESObjectREFR* a_this) {
        float result = _GetScaleJumpHook(a_this);
        Actor* actor = skyrim_cast<Actor*>(a_this);
        if (actor) {
          float scale = get_visual_scale(actor);
          if (scale > 1e-4) {
            log::info("Jump Hook: {} for {}", scale, actor->GetDisplayFullName());
            result *= scale;
          }
        }
        return result;
  }
}
