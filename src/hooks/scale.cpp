#include "hooks/scale.hpp"
#include "scale/scale.hpp"
#include "hooks/callhook.hpp"

using namespace RE;
using namespace SKSE;

namespace Hooks {

	void Hook_Scale::Hook(Trampoline& trampoline) {
    // AE 1402bc7c3
    // SE 1402aa40c
    static CallHook<float(TESObjectREFR*)> GetScaleHook1(RELOCATION_ID(19889, 20296),  REL::Relocate(0xbc, 0xbc, 0x493),
    [](auto* self) {
        float result = GetScaleHook1(self);
        Actor* actor = skyrim_cast<Actor*>(self);
        if (actor) {
          float scale = get_visual_scale(actor);
          if (scale > 1e-4) {
            result *= scale;
          }
        }
        return result; // Hook inverts the return value;
    });
	}
}
