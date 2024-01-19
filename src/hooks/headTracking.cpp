#include "hooks/headTracking.hpp"
#include "utils/actorUtils.hpp"
#include "scale/scale.hpp"

using namespace RE;
using namespace SKSE;

namespace {
	void SetHeadtrackTargetImpl(Actor* actor, NiPoint3& target) {
		if (!actor) {
			return;
		}
		if (IsHeadtracking(actor)) { // Needed to fix TDM bugs with deforming Meshes of Actors when we lock onto someone
			return;
		}
		// log::info("Actor: {}", actor->GetDisplayFullName());
		auto headPos = actor->GetLookingAtLocation();
		// log::info("headPos: {}", Vector2Str(headPos));
		auto model = actor->Get3D();
		if (!model) {
			return;
		}
		auto trans = model->world;
		auto transInv = trans.Invert();
		auto scale = get_visual_scale(actor);

		// log::info("headPos (local): {}", Vector2Str(transInv*headPos));
		auto unscaledHeadPos = trans * (transInv*headPos * (1.0/scale));
		// log::info("unscaledHeadPos: {}", Vector2Str(unscaledHeadPos));
		// log::info("unscaledHeadPos (local): {}", Vector2Str(transInv*headPos));
		auto direction = target - headPos;
		// log::info("direction: {}", Vector2Str(direction));
		target = unscaledHeadPos + direction;
	}
}

namespace Hooks
{

	void Hook_HeadTracking::Hook(Trampoline& trampoline) {
		/*static FunctionHook<void(AIProcess* a_this, Actor* a_owner, NiPoint3& a_targetPosition)> SetHeadtrackTarget(RELOCATION_ID(38850, 39887),
				[](auto* a_this, auto* a_owner, auto& a_targetPosition) {
				SetHeadtrackTargetImpl(a_owner, a_targetPosition);
				SetHeadtrackTarget(a_this, a_owner, a_targetPosition);
				return;
			}
		);*/

		static FunctionHook<float(Actor* actor)> GetEyeHeight_140601E40(
			REL::RelocationID(36845, 36845),
			[](auto* actor){
				float result = GetEyeHeight_140601E40(actor);
				if (actor) {
					if (actor->formID == 0x14 || IsTeammate(actor)) {
						float scale = get_visual_scale(actor);
						log::info("Unscaled Eye Height of {} is {}", actor->GetDisplayFullName(), result);
						result *= scale;
						log::info("Scaled Eye Height of {} is {}, scale: {}", actor->GetDisplayFullName(), result, scale);
					}
				}
				return result;
			}
		);
	}
}
