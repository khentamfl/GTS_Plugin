#include "utils/actorUtils.hpp"
#include "hooks/callhook.hpp"
#include "hooks/Stealth.hpp"
#include "scale/scale.hpp"
#include "utils/debug.hpp"


using namespace RE;
using namespace SKSE;

namespace {
    float modify_detection() {
        auto player = PlayerCharacter::GetSingleton();
        float modify = get_visual_scale(player);
        return modify;
    }
    float modify_footstep_detection(Actor* giant, float in) {
        float scale = get_visual_scale(giant);
        float modify = 0.0;
        if (HasSMT(giant)) {
            modify = (in * 4.0 + 80) * scale;
        } else {
            if (in > 0) {
                modify = in * (scale * scale * scale);
            } else {
                modify = 1.0 * (scale * scale * scale);
                modify -= 1.0;
            }
        }
        return modify;
    }
}

namespace Hooks {

	void Hook_Stealth::Hook(Trampoline& trampoline) { 
        
        // NEEDS AE OFFSET AND REL!
        /*static CallHook<float(Actor* giant)>CalculateFootstepDetection_1405FD870_5D0(
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
        );*/

        static CallHook<NiAVObject*(Actor* giant, NiPoint3* param_1, NiPoint3* param_2, float param_3)>CalculateLOS(
			REL::RelocationID(36758, 36758), REL::Relocate(0x6C5, 0x6C5), 
            //  0x1405FD870 (func) - 0x1405fdf35 (LOS) = -0x6C5 (just remove -)
            // altering CalculateLOS_1405FD2C0
			[](auto* giant, auto* param_1, auto* param_2, float param_3) {
				log::info("-- LOS Result for {}", giant->GetDisplayFullName());
                log::info("-------LOS param_1: {}", Vector2Str(param_1));
                log::info("-------LOS param_2: {}", Vector2Str(param_2));
                log::info("-------LOS param_3: {}", param_3);
				return CalculateLOS(giant, param_1, param_2, param_3);
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
        );

        static CallHook<float(Actor* giant, NiPoint3* param_1)>CalculateHeading_var2(
			REL::RelocationID(36758, 36758), REL::Relocate(0x92D, 0x92D), 
            //  0x1405fe19d - 0x1405FD870 = 0x92D (line 370)
            //  altering Character::GetHeading_1405FD780
			[](auto* giant, auto* param_1) {
				log::info("-- Heading_2 Result for {}", giant->GetDisplayFullName());
                log::info("-------Heading_2 param_1: {}", Vector2Str(param_1));
                log::info("-------Heading_2 Result: {}", CalculateHeading_var2(giant, param_1));
				return CalculateHeading_var2(giant, param_1);
            }
        );

        static CallHook<float(Actor* giant, NiPoint3* param_1)>CalculateHeading_var3(
			REL::RelocationID(36758, 36758), REL::Relocate(0x217, 0x217), 
            //  0x1405fda87 - 0x1405FD870 = 0x217 (line ~150)
            //  altering Character::GetHeading_1405FD780
			[](auto* giant, auto* param_1) {
				log::info("-- Heading 3 Result for {}", giant->GetDisplayFullName());
                log::info("-------Heading 3 param_1: {}", Vector2Str(param_1));
                log::info("-------Heading 3 Result: {}", CalculateHeading_var3(giant, param_1));
				return CalculateHeading_var3(giant, param_1);
            }
        );*/


        


       static FunctionHook<float(Actor* giant, uintptr_t param_2,uintptr_t param_3,uintptr_t param_4, uintptr_t param_5,
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