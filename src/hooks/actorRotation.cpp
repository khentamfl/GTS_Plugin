#include "hooks/actorRotation.hpp"
#include "data/transient.hpp"
#include "hooks/callhook.hpp"
#include "scale/scale.hpp"
#include "data/plugin.hpp"


using namespace RE;
using namespace SKSE;

namespace {
    float GetTinyRotation_X(Actor* actor) {
        float rotation_x = 0.0;
        auto transient = Transient::GetSingleton().GetData(actor);
        if (transient) {
            rotation_x = transient->Rotation_X;
        }
        return rotation_x;
    }
}

namespace Hooks {

	void Hook_ActorRotation::Hook(Trampoline& trampoline) {

        static FunctionHook<float(Actor* actor)> Skyrim_GetActorRotationX(  // 36601 = 1405EDD40 (SE), AE = ???
            REL::RelocationID(36601, 36601),
            [](auto* actor) {
                float result = Skyrim_GetActorRotationX(actor);
                float transient_rotation = GetTinyRotation_X(actor);
                if (transient_rotation != 0.0) {
                    result = transient_rotation;
                    log::info("Rotation != 0");
                }
                
                log::info("Rotation X of {} is: {}", actor->GetDisplayFullName(), result);
                
                
                return result;
            }
        );
     }
}