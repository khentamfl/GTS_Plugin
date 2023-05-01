#include "hooks/headTracking.hpp"

using namespace RE;
using namespace SKSE;

namespace Hooks
{
  static FunctionHook<void(AIProcess* a_this, Actor* a_owner, NiPoint3& a_targetPosition)> SetHeadtrackTarget(RELOCATION_ID(38850, 39887)),
    [](auto* self, auto* owner, auto& target) {
        log::info("Function HOOKED: {}, {}", owner? owner->GetDisplayFullName() : "", Vector2Str(taget));
        SetHeadtrackTarget(self, owner, target);
        return;
    });
}
