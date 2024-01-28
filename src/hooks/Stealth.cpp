#include "utils/actorUtils.hpp"
#include "hooks/callhook.hpp"
#include "hooks/Stealth.hpp"
#include "scale/scale.hpp"
#include "utils/debug.hpp"


using namespace RE;
using namespace SKSE;

namespace {
    float modify_detection(float in) {
        float modify = 1.0;
        if (in > 1e-6) {
            auto player = PlayerCharacter::GetSingleton();
            float scale = get_visual_scale(player);
            if (scale > 1.0) {
                modify = 1.0 / scale;
            } else if (scale < 1.0) {
                modify = 1.0 / (scale * scale); // more efficient for < 1.0 scales
            }
            
        }
        return modify;
    }
    float modify_footstep_detection(Actor* giant, float in) {
        float scale = get_visual_scale(giant);
        float massscale = (scale * scale * scale);
        float modify = 0.0;
        if (HasSMT(giant)) {
            modify = (in * 4.0 + 80) * scale;
        } else {
            if (in > 1e-6) {
                modify = in * massscale;
            } else {
                modify = (1.0 * massscale) - 1.0;
            }
        }

        if (scale < 1.0) {
            modify += in * massscale;
            modify -= 1.0 / (scale * scale * scale); // harder to hear small player
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
					log::info("New result: {}", alter);
					result = alter;
				}
				return result;
            }
        );

       /* static CallHook<float(Actor* giant, NiPoint3* param_1)>CalculateHeading(
			REL::RelocationID(36758, 36758), REL::Relocate(0x71E, 0x71E), 
            //  0x1405fe19d - 0x1405FD870 = 0x71E (line 296)
            //  altering Character::GetHeading_1405FD780
			[](auto* giant, auto* param_1) {
				log::info("-- Heading Result for {}", giant->GetDisplayFullName());
                log::info("-------Heading param_1: {}", Vector2Str(param_1));
                log::info("-------Heading Result: {}", CalculateHeading(giant, param_1));
				return CalculateHeading(giant, param_1);
            }
        );*/

        static CallHook<float(Actor* giant, NiPoint3* param_1)>CalculateHeading_var2(
			REL::RelocationID(36758, 36758), REL::Relocate(0x92D, 0x92D), 
            //  0x1405fe19d - 0x1405FD870 = 0x92D (line 370)
            //  altering Character::GetHeading_1405FD780
			[](auto* giant, auto* param_1) {
				log::info("-- Heading_2 Result for {}", giant->GetDisplayFullName());
                float result = CalculateHeading_var2(giant, param_1);
                log::info("-------Heading_2 Result: {}", CalculateHeading_var2(giant, param_1));
                result *= modify_detection(result);
                log::info("-------Heading_2 Result Alter: {}", CalculateHeading_var2(giant, param_1));
				return CalculateHeading_var2(giant, param_1);
            }
        );

        static CallHook<float(Actor* giant, NiPoint3* param_1)>CalculateHeading_var3(
			REL::RelocationID(36758, 36758), REL::Relocate(0x217, 0x217), 
            //  0x1405fda87 - 0x1405FD870 = 0x217 (line ~150)
            //  altering Character::GetHeading_1405FD780
			[](auto* giant, auto* param_1) {
				log::info("-- Heading 3 Result for {}", giant->GetDisplayFullName());
                float result = CalculateHeading_var3(giant, param_1);
                log::info("-------Heading 3 Result: {}", result);
                result *= modify_detection(result);
                log::info("-------Heading 3 Result Post: {}", result);
				return result;
            }
        );

      /* static FunctionHook<float(Actor* giant, uintptr_t param_2,uintptr_t param_3,uintptr_t param_4, uintptr_t param_5,
			uintptr_t param_6, uintptr_t param_7, uintptr_t param_8, uintptr_t param_9, uintptr_t param_10)>
            CalculateDetection_1405FD870( REL::RelocationID(36758, 36758),
			[](auto* giant, auto param_2, auto param_3, auto param_4, auto param_5, auto param_6, auto param_7, auto param_8, auto param_9, auto param_10) {
                if (giant->formID == 0x14 || IsTeammate(giant)) {
                    log::info("- Hook Results for {}", giant->GetDisplayFullName());
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

                float result = CalculateDetection_1405FD870(giant, param_2, param_3, param_4, param_5, param_6, param_7, param_8, param_9, param_10);
                result = 0.0;
				log::info("Hook Result: {}", result);
				return result;
            }
        );*/ // The general stealth hook.

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