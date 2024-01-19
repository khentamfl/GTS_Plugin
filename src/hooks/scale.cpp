#include "hooks/scale.hpp"
#include "scale/scale.hpp"
#include "hooks/callhook.hpp"

using namespace RE;
using namespace SKSE;


// Possible hooks that may benefit from scaling: All info for Special Edition (SE)
//     [x] = attempted to hook it
// -[x] Character::GetEyeHeight_140601E40  									(Not headtracking. F.)  
// -[x] Actor::Jump_1405D1F80              									Jump Height. We already hook that.
// -[x] Pathing::sub_140474420             									(perhaps pathing fix)  [Looks like it does nothing, there's no prints]
// -[x] TESObjectREFR::sub_140619040       									 Offset:  0x17E        37323     
// -[x] IAnimationGraphManagerHolder::Func7_140609D50      					 Offset:  0xBD
// - FUN_1405513a0                                      					(Something attack angle related)
// -[x] NiNode::sub_1402AA350(NiNode *param_1)                           	 Offset:  0xBC
// - TESObject::LoadGraphics_140220DD0(TESObject *param_1)                   Offset:  0x1FC
// -[x] TESObjectREFR::sub_1407BA9C0                                            offset:  0x57         46015     
// -[x] TESObjectREFR::sub_1407BA910                                            offset:  0x94         46014 
// -[x] TESObjectREFR::sub_1407BA8B0                                            offset:  0x44         46013  
//    ^ These 3 subs seem to do literally nothing.
//      TESObjectREFR::sub_1406AA410                                           offset: 0xC1           39477

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
		/*static FunctionHook<void(uintptr_t* param_1, uintptr_t param_2, uintptr_t param_3, uintptr_t param_4, uintptr_t param_5)> Skyrim_Pathing_140474420( 
			// This hook seems to do nothing.
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
        );*/

		/*static CallHook<float(NiNode* node)> Skyrim_NiNode(RELOCATION_ID(19889, 19889),  REL::Relocate(0xBC, 0xBC), // Prints nothing
		[](auto* node) {
		    float result = Skyrim_NiNode(node);
			log::info("Original Node Value: {}", result);
		    if (node) {
				float scale = 10.0;
				result *= scale;
				
				log::info("Node Value: {}", result);
		    }
		    return result;
		});*/

		/*static CallHook<float(TESObjectREFR* ref)> Skyrim_sub_140619040(RELOCATION_ID(37323, 37323),  REL::Relocate(0x17E, 0x17E), // Prints stuff but unsure what it does. 
		// Seems to be applied when new objects (actors?) are loaded into the scene, almost always reports negative value after * scale
		[](auto* ref) {
			// No idea what it does, rarely prints something.
		    float result = Skyrim_sub_140619040(ref);
			log::info("Original Ref Value: {}", result);
		    if (ref) {
				Actor* giant = skyrim_cast<Actor*>(ref);
				if (giant) {
					float scale = get_visual_scale(giant);
					result *= scale;
				}
				
				log::info("Ref New Scale: {}", result);
		    }
		    return result;
		});

		static CallHook<float(IAnimationGraphManagerHolder* graph)> Skyrim_AnimGraph_140609D50(RELOCATION_ID(36957, 36957),  REL::Relocate(0xBD, 0xBD),
		[](auto* graph) {
		    float result = Skyrim_AnimGraph_140609D50(graph);
			log::info("Original Graph Value: {}", result);
			Actor* giant = skyrim_cast<Actor*>(graph);
			if (giant) {
				if (giant->formID == 0x14 || IsTeammate(giant)) {
					float scale = get_visual_scale(giant);
					if (scale > 0.0) {
						result *= scale;
						log::info("Found Actor: {}, scale: {}", giant->GetDisplayFullName(), scale);
					}

				log::info("Graph New Scale: {}", result);
				}
			}
		    return result;
		});*/

		static CallHook<float(TESObjectREFR* ref, uintptr_t param_2, uintptr_t param_3, uintptr_t Character, uintptr_t param_5)> 
		Skyrim_Ref_sub_1406AA410(RELOCATION_ID(39477, 39477),  REL::Relocate(0xC1, 0xC1),
		[](auto* ref, auto param_2, auto param_3, auto Character, auto param_5) {
		    float result = Skyrim_Ref_sub_1406AA410(ref);
			log::info("Sub A410: Original Ref_Sub Value: {}", result);
			log::info("Param 1 Raw Name: {}", GetRawName(ref));
			log::info("Param 2 Raw Name: {}", GetRawName(param_2));
			log::info("Param 3 Raw Name: {}", GetRawName(param_3));
			log::info("Param 4 Raw Name: {}", GetRawName(Character));
			log::info("Param 5 Raw Name: {}", GetRawName(param_5));
			Actor* giant = skyrim_cast<Actor*>(ref);
			if (giant) {
				if (giant->formID == 0x14 || IsTeammate(giant)) {
					float scale = get_visual_scale(giant);
					if (scale > 0.0) {
						result *= scale;
						log::info("Found Actor: {}, scale: {}", giant->GetDisplayFullName(), scale);
					}

				log::info("Sub A410: New Ref Sub Value: {}", result);
				}
			}
		    return result;
		});
	}
}
