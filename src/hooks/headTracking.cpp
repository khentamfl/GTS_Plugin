#include "hooks/headTracking.hpp"

using namespace RE;
using namespace SKSE;

namespace {
  void SetHeadtrackTargetImpl(Actor* actor, NiPoint3& target) {
    if (!actor) {
      return;
    }
    auto headPos = actor->GetLookingAtLocation();
    auto scale = get_visual_scale(actor);
    auto location = actor->GetPosition();
    auto unscaledHeadPos = (headPos - location) * (1.0/scale) + location;
    auto direction = target - unscaledHeadPos;
    target = headPos + direction;
  }
}

namespace Hooks
{

  void Hook_HeadTracking::Hook(Trampoline& trampoline) {
    static FunctionHook<void(AIProcess* a_this, Actor* a_owner, NiPoint3& a_targetPosition)> SetHeadtrackTarget(RELOCATION_ID(38850, 39887),
      [](auto* a_this, auto* a_owner, auto& a_targetPosition) {
        NiPoint3 newPoint = a_targetPosition;
        SetHeadtrackTargetImpl(a_outWorldOrCell, newPoint);
        SetHeadtrackTarget(a_this, a_owner, newPoint);
        return;
      }
    );
  }
}
