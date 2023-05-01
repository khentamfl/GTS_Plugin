#include "hooks/headTracking.hpp"

using namespace RE;
using namespace SKSE;

namespace Hooks
{

  void Hook_HeadTracking::Hook(Trampoline& trampoline) {
    // Hook Test
    static FunctionHook<void(AIProcess* a_this, Actor* a_owner, NiPoint3& a_targetPosition)> SetHeadtrackTarget(RELOCATION_ID(38850, 39887),
      [](auto* a_this, auto* a_owner, auto& a_targetPosition) {
        log::info("Function HOOKED: {}, {}", a_owner? a_owner->GetDisplayFullName() : "", Vector2Str(taget));
        SetHeadtrackTarget(a_this, a_owner, a_targetPosition);
        return;
      }
    );
  }
}
