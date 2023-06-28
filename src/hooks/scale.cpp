#include "hooks/scale.hpp"
#include "scale/scale.hpp"
#include "hooks/callhook.hpp"
#include "hooks/functionhook.hpp"

using namespace RE;
using namespace SKSE;

namespace Hooks {

	void Hook_Scale::Hook(Trampoline& trampoline) {
    // AE 1402bc7c3
    // SE 1402aa40c
    //
    // Seems to be unused
    //     fVar1 = TESObjectREFR::GetScale_14028CC60(param_2);
    //     (param_3->LocalTransform_48).Scale_30 = (float)((uint)(fVar1 * fVar2) & 0x7fffffff);
    //
    //     It sets the NiNode scale to match that of an GetScale Call
    //
    // static CallHook<float(TESObjectREFR*)> GetScaleHook1(RELOCATION_ID(19889, 20296),  REL::Relocate(0xbc, 0xbc, 0x493),
    // [](auto* self) {
    //     float result = GetScaleHook1(self);
    //     Actor* actor = skyrim_cast<Actor*>(self);
    //     if (actor) {
    //       float scale = get_visual_scale(actor);
    //       if (scale > 1e-4) {
    //         log::info("Scale Hook: {} for {}", scale, actor->GetDisplayFullName());
    //         result *= scale;
    //       }
    //     }
    //     return result;
    // });

    // SE: 140290bf8
    // Used during Set3D

    static FunctionHook<float(TESObjectREFR*)> GetScale(RELOCATION_ID(19238, 19664),
      [](auto* self) {
        float result = GetScale(self);
        Actor* actor = skyrim_cast<Actor*>(self);
        if (actor) {
          // float scale = get_giantess_scale(actor);
          // result *= scale;
          if (actor->formID == 0x14) {
            result = 2.5;
          }
        }
        return result;
      }
    );


	}
}
