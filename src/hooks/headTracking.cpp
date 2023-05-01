#include "hooks/headTracking.hpp"
#include "scale/scale.hpp"

using namespace RE;
using namespace SKSE;

namespace {
  NiPoint3 SetHeadtrackTargetImpl(Actor* actor, const NiPoint3& target) {
    if (!actor) {
      return NiPoint3();
    }
    auto unscaledHeadPos = actor->GetLookingAtLocation();
    log::info("unscaledHeadPos: {}", Vector2Str(unscaledHeadPos));
    auto scale = get_visual_scale(actor);
    auto location = actor->GetPosition();
    log::info("unscaledHeadPos - location: {}", Vector2Str(unscaledHeadPos - location));
    auto headPos = (unscaledHeadPos - location) * (scale) + location;
    log::info("headPos: {}", Vector2Str(headPos));
    log::info("headPos - location: {}", Vector2Str(headPos - location));
    auto direction = target - headPos;
    log::info("direction: {}", Vector2Str(direction));
    return unscaledHeadPos + direction;
  }
}

namespace Hooks
{

  void Hook_HeadTracking::Hook(Trampoline& trampoline) {
    static FunctionHook<void(AIProcess* a_this, Actor* a_owner, NiPoint3& a_targetPosition)> SetHeadtrackTarget(RELOCATION_ID(38850, 39887),
      [](auto* a_this, auto* a_owner, auto& a_targetPosition) {
        NiPoint3 newPoint = SetHeadtrackTargetImpl(a_owner, a_targetPosition);
        SetHeadtrackTarget(a_this, a_owner, newPoint);
        return;
      }
    );
  }
}
