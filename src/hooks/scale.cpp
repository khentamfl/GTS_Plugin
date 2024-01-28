#include "utils/actorUtils.hpp"
#include "hooks/callhook.hpp"
#include "data/transient.hpp"
#include "hooks/scale.hpp"
#include "scale/scale.hpp"


using namespace RE;
using namespace SKSE;


// Possible hooks that may benefit from scaling: All info for Special Edition (SE)
//  [x] = attempted to hook it
// -[x] Character::GetEyeHeight_140601E40  									(Barely prints anything with unknown conditions. Useless)  
// -[x] Actor::Jump_1405D1F80              									Jump Height. We already hook that.
// -[x] Pathing::sub_140474420             									(perhaps pathing fix)  [Looks like it does nothing, there's no prints]
// -[x] TESObjectREFR::sub_140619040       									 Offset:  0x17E        37323     
// -[x] IAnimationGraphManagerHolder::Func7_140609D50      					 Offset:  0xBD
// - FUN_1405513a0                                      					(Something attack angle related)
// -[x] NiNode::sub_1402AA350(NiNode *param_1)                           	 Offset:  0xBC
// - TESObject::LoadGraphics_140220DD0(TESObject *param_1)                   Offset:  0x1FC
// -[x] TESObjectREFR::sub_1407BA9C0                                         offset:  0x57         46015     
// -[x] TESObjectREFR::sub_1407BA910                                         offset:  0x94         46014 
// -[x] TESObjectREFR::sub_1407BA8B0                                         offset:  0x44         46013  
// -[x] TESObjectREFR::sub_1406AA410                                         offset:  0xC1         39477
// ^ These 4 subs seem to do literally nothing.

//      TESObjectREFR::sub_1405FE650
//      Pathing::sub_140473120(Pathing *param_1,uint64 param_2,Character *param_3,uint64 param_4) (Seems to be called in lots of places
//      ^ 29819
//      Pathing::sub_140473490(Pathing *param_1,uintptr_t param_2,uintptr_t param_3,uintptr_t param_4, uint64 param_5)
//      ^ 29824
//      Pathing::sub_140473200(Pathing *param_1,uint64 param_2,Character *param_3,uint64 param_4)
//      ^ 29820
//      PlayerCamera::Update_14084AB90
//      FUN_14085c290(uint64 param_1,char *param_2,uint64 param_3,uint64 param_4)
//      ^ 50179

namespace {
	float camera_getplayersize() {
		auto player = PlayerCharacter::GetSingleton();
		if (player) {
			float size = get_giantess_scale(player);
			return size;
		}
		return 1.0;
	}

	NiPoint3* camera_getplayeroffset() {
		auto player = PlayerCharacter::GetSingleton();
		if (player) {
			float scale = get_visual_scale(player);
			float ns = get_natural_scale(player);
			if (scale > 0) {
				float offset = 70.0 * (scale - ns);

				NiPoint3 in = NiPoint3(offset, 0.0, offset);

				NiPoint3& adjust = in;
				return &adjust;
			}
		}
		NiPoint3 in = NiPoint3(0.0, 0.0, 0.0);
		NiPoint3& adjust = in;
		return &adjust;
	}
}

namespace Hooks {

	void Hook_Scale::Hook(Trampoline& trampoline) { // This hook is commented out inside hooks.cpp

		

		

		/*static CallHook<float(Actor* param_1, uintptr_t param_2,uintptr_t param_3,uintptr_t param_4, uintptr_t param_5,
			uintptr_t param_6, uintptr_t param_7, uintptr_t param_8, uintptr_t param_9, uintptr_t param_10)>CalculateDetection_1405FD870(
			REL::RelocationID(36758, 36758), REL::Relocate(0xE0, 0xE0),
			[](auto* param_1, auto param_2, auto param_3, auto param_4, auto param_5, auto param_6, auto param_7, auto param_8, auto param_9, auto param_10) {
				float result = CalculateDetection_1405FD870(param_1, param_2, param_3, param_4, param_5, param_6, param_7, param_8, param_9, param_10);
				log::info("Hook Result for {} is {}", param_1->GetDisplayFullName(), result);
				return result;
            }
        );*/

		/*static FunctionHook<void(TESObjectREFR* ref, float X)>Skyrim_SetAngleX(  
            REL::RelocationID(19360, 19360), // 19360 = 140296680
            [](auto* ref, auto X) {
				Actor* actor = skyrim_cast<Actor*>(ref);
				if (actor) {
					if (actor->formID != 0x14 && IsTeammate(actor)) {
						//X = random;
						log::info("Value post: {}", X);
					}
				}
                return Skyrim_SetAngleX(ref, X);
				// rotation is capped between 1.57 and - 1.57 based on prints
            }
        );

		static FunctionHook<void(Actor* ref, float X)>Skyrim_SetRotationX( 
            REL::RelocationID(36602, 37610),
            [](auto* ref, auto X) {
				if (ref->formID != 0x14 && IsTeammate(ref)) {
					log::info("- SetRotation X is called for {}", ref->GetDisplayFullName());
					log::info("- Value: {}", X);
				}
				
                return Skyrim_SetRotationX(ref, X);
            }
        );*/

		static FunctionHook<NiPoint3*(NiCamera* camera)> Skyrim_Camera(  // camera hook works just fine that way
            REL::RelocationID(69271, 70641),
            [](auto* camera) {
				//log::info("Camera hook is running");
				auto result = Skyrim_Camera(camera);
				log::info("Hook Result: {}", Vector2Str(result));
				result += camera_getplayeroffset();
				log::info("Hook Result After: {}", Vector2Str(result));
                return result;
            }
        );

		// Yet the one below CTD's. Sigh.
		/*static CallHook<float(const NiCamera* camera)> Skyrim_Camera_posX(RELOCATION_ID(69271, 70641),  REL::Relocate(0x11, 0x11), // ctd, ctd and ctd.
		[](const NiCamera* camera) { // 0x140C66710 - 0x140c66b70 (fVar18) = 0x11 . No AE rel
			log::info("Trying to hook camera");
			//auto result = Skyrim_Camera_posX(camera);
			log::info("Hooked camera");
			double result = 0.0;
			log::info("Pos X is 0");
			return result;
		});*/

		/*static CallHook<float(uintptr_t* cam)> Skyrim_Camera_posY(RELOCATION_ID(69271, 70641),  REL::Relocate(0x64, 0x64),
		[](auto* cam) { // 0x140C66710 - 0x140c66774 (fVar14) = 0x64 . No AE rel.
			if (cam) {
				float result = Skyrim_Camera_posY(cam);
				log::info("Pos Y: {}", result);
				return result;
			}
		});

		static CallHook<float(uintptr_t* cam)> Skyrim_Camera_posZ(RELOCATION_ID(69271, 70641),  REL::Relocate(0x19, 0x19),
		[](auto* cam) { // 0x140C66710 - 0x140c66729 (fVar15) = 0x19 . No AE rel.
			if (cam) {
				float result = Skyrim_Camera_posZ(cam);
				//log::info("Pos Z: {}", result);
				return result;
			}
		});*/

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

		/*static FunctionHook<void(uintptr_t* param_1, uintptr_t param_2, uintptr_t param_3, uintptr_t param_4)> Skyrim_Pathing_140473120( 
            REL::RelocationID(29819, 29819),
            [](auto* param_1, auto param_2, auto param_3, auto param_4) {
                
				log::info("3120: Param 1: {}", GetRawName(param_1)); 
				log::info("3120: Param 2: {}", param_2); 
				log::info("3120: Param 3: {}", param_3); 
				log::info("3120: Param 4: {}", param_4); 

                return Skyrim_Pathing_140473120(param_1, param_2, param_3, param_4);
            }
        );

		static FunctionHook<void(uintptr_t* param_1, uintptr_t param_2, uintptr_t param_3, uintptr_t param_4, uintptr_t param_5)> Skyrim_Pathing_140473490( 
            REL::RelocationID(29824, 29824),
            [](auto* param_1, auto param_2, auto param_3, auto param_4, auto param_5) {
                
				log::info("3490: Param 1: {}", GetRawName(param_1)); 
				log::info("3490: Param 2: {}", param_2); 
				log::info("3490: Param 3: {}", param_3); 
				log::info("3490: Param 4: {}", param_4); 
				log::info("3490: Param 5: {}", param_5); 

                return Skyrim_Pathing_140473490(param_1, param_2, param_3, param_4, param_5);
            }
        );

		static FunctionHook<void(uintptr_t* param_1, uintptr_t param_2, uintptr_t param_3, uintptr_t param_4)> Skyrim_Pathing_140473200( 
            REL::RelocationID(29820, 29820),
            [](auto* param_1, auto param_2, auto param_3, auto param_4) {
                
				log::info("3200: Param 1: {}", GetRawName(param_1)); 
				log::info("3200: Param 2: {}", param_2); 
				log::info("3200: Param 3: {}", param_3); 
				log::info("3200: Param 4: {}", param_4); 

                return Skyrim_Pathing_140473200(param_1, param_2, param_3, param_4);
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
	}
}
