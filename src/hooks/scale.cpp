#include "hooks/scale.hpp"
#include "scale/scale.hpp"
#include "hooks/callhook.hpp"

using namespace RE;
using namespace SKSE;


// Possible hooks that may benefit from scaling: (SE)
// - Character::GetEyeHeight_140601E40  (Headtracking?)
// - Actor::Jump_1405D1F80              (Maybe jump height)
// - Pathing::sub_140474420             (perhaps pathing fix)
// - TESObject::LoadGraphics_140220DD0  (Model render distance?)


namespace Hooks {

	void Hook_Scale::Hook(Trampoline& trampoline) { // This hook is commented out inside hooks.cpp
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

		/*static FunctionHook<float(TESObjectREFR* ref)> Skyrim_GetScale_14028CC60(  // 19238 = 14028CC60 (SE), AE = ???
		    // Very unstable hook to say the least. Does same effect as SetScale() command (without fps cost)
			// Seems to multiply actor speed and causes chaos in-game overall
			// Actors fly and behave weirdly. The only positive effect of this hook is fixed Headtracking and possible Pathing improvements.
            REL::RelocationID(19238, 19238),
            [](auto* ref) {
                float result = Skyrim_GetScale_14028CC60(ref);
                Actor* actor = skyrim_cast<Actor*>(ref);
                if (actor) {
					float scale = get_visual_scale(actor);
                    result *= scale;
                    log::info("Scale Hook: {} for {}", scale, actor->GetDisplayFullName());
                }

                return result;
            }
        );*/

									//(Pathing *param_1,undefined param_2,undefined param_3,undefined param_4, uint64 param_5)
		static FunctionHook<void(uintptr_t* param_1, uintptr_t param_2, uintptr_t param_3, uintptr_t param_4, uintptr_t param_5)> Skyrim_Pathing_140474420( 
			 // 29837 = 140474420 (SE), AE = ???
            REL::RelocationID(29837, 29837),
            [](auto* param_1, auto param_2, auto param_3, auto param_4, auto param_5) {
                
				log::info("Param 1: {}", GetRawName(param_1)); 
				log::info("Param 2: {}", param_2); 
				log::info("Param 3: {}", param_3); 
				log::info("Param 4: {}", param_4); 
				log::info("Param 5: {}", param_5); 

                return Skyrim_Pathing_140474420(param_1, param_2, param_3, param_4, param_5);
            }
        );
	}
}
