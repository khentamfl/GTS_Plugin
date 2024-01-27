#include "utils/actorUtils.hpp"
#include "hooks/callhook.hpp"
#include "hooks/Stealth.hpp"
#include "scale/scale.hpp"


using namespace RE;
using namespace SKSE;

namespace {
    float modify_footstep_detection(Actor* giant, float in) {
        float scale = get_visual_scale(giant);
        float modify = 0.0;
        if (HasSMT(giant)) {
            modify = in * 4 + (120.0 * scale);
        } else {
            if (in > 0) {
                modify = in * (scale * scale * scale);
            } else {
                modify = 1.0 * (scale * scale * scale) - 1.0;
            }
        }
        return modify;
    }
}

namespace Hooks {

	void Hook_Stealth::Hook(Trampoline& trampoline) { 
        
        // NEEDS AE OFFSET AND REL!
        static CallHook<float(Actor* giant)>CalculateFootstepDetection_1405FD870_5D0(
			REL::RelocationID(36758, 36758), REL::Relocate(0x2D4, 0x2D4), 
            //  0x1405FD870 (func) - 0x1405fdb44 (weight) = -0x2D4 (just remove -)
            // altering Character::GetEquippedWeight_1406195D0
			[](auto* giant) {
				float result = CalculateFootstepDetection_1405FD870_5D0(giant); // Makes footsteps lounder for AI, works nicely so far
				if (giant->formID == 0x14 || IsTeammate(giant)) {
					log::info("Hook Weight Result for {} is {}", giant->GetDisplayFullName(), result);
					float alter = modify_footstep_detection(giant, result);
					log::info("New result: {}", result);
					result = alter;
				}
				return result;
            }
        );


        static FunctionHook<void(Actor* giant, uintptr_t param_2,uintptr_t param_3,uintptr_t param_4, uintptr_t param_5,
			uintptr_t param_6, uintptr_t param_7, uintptr_t param_8, uintptr_t param_9, uintptr_t param_10)>
            CalculateDetection_1405FD870( REL::RelocationID(36758, 36758),
			[](auto* giant, auto param_2, auto param_3, auto param_4, auto param_5, auto param_6, auto param_7, auto param_8, auto param_9, auto param_10) {
                if (giant->formID == 0x14 || IsTeammate(giant)) {
                    log::info("- Hook Results for {}", giant->GetDisplayFullName());
                    param_2 = 0.0;
                    param_3 = 0.0;
                    param_4 = 0.0;
                    log::info("------ Param_2 {}", param_2);
                    log::info("------ Param_3 {}", param_3);
                    log::info("------ Param_4 {}", param_4);
                    log::info("------ Param_5 {}", param_5);
                    log::info("------ Param_6 {}", param_6);
                    log::info("------ Param_7 {}", param_7);
                    log::info("------ Param_8 {}", param_8);
                    log::info("------ Param_9 {}", param_9);
                    log::info("------ Param_10 {}", param_10);
                }
				
				return CalculateDetection_1405FD870(giant, param_2, param_3, param_4, param_5, param_6, param_7, param_8, param_9, param_10);
            }
        ); // The general stealth hook.

        /*static FunctionHook<float(Actor* ref)>GetDetectionCalculatedValue( 
            REL::RelocationID(36748, 36748),
            [](auto* ref) {
				float result = 0.0;//GetDetectionCalculatedValue(ref);

				log::info("Detection of {} is {}", ref->GetDisplayFullName(), result);
				
                return result;
            }
        );*/// works but unknown what it does
    }
}