#include "hooks/headTracking.hpp"
#include "utils/actorUtils.hpp"
#include "scale/scale.hpp"

using namespace RE;
using namespace SKSE;


namespace {
	float affect_by_scale(TESObjectREFR* ref, float original) {
		Actor* giant = skyrim_cast<Actor*>(ref);
		if (giant) {
			if (IsRagdolled(giant)) { // For some reason breaks tiny ragdoll when they're small, so they fly into the sky.
				return original;      // So this check is a must because of that bug.
			}
			return get_giantess_scale(giant);
		}
		return original;
	}
}

namespace Hooks
{

	void Hook_HeadTracking::Hook(Trampoline& trampoline) {
		static CallHook<float(TESObjectREFR* param_1)>Alter_Headtracking( 
			REL::RelocationID(37129, 37356), REL::Relocate(0x24, 0x295),
			[](auto* param_1) {
				// SE:
				// 37129
				// 0x140615054 - 0x140615030 = 0x24
				// FUN_140615030

				//------------------ AE:
				// FUN_1407ea630 ? 
				// FUN_1407e65a0
				// FUN_1407e6360
				// FUN_14078e130
				// FUN_14078e060
				// FUN_14074f230
				// FUN_140746b40

				// FUN_140637dd0  38041
				// or
				// FUN_1405ffc50 37364

				
				// FUN_1405fe580 37356  (A correct one?)
				// 0x140637def - 0x140637dd0 = 0x1F
				// 0x1402a3c30 = GetNiNode

				// 0x1405fe815 - 0x1405fe580 = 0x295
  
				float result = Alter_Headtracking(param_1);
				float Alter = 10.0; //affect_by_scale(param_1, result);
				//log::info("(20) Alter_Headtracking Hooked");
				return Alter;
            }
        );

		/*static CallHook<float(TESObjectREFR* param_1)>GetEyeHeight_140601E40(  // Get Eye Height, rarely called
			REL::RelocationID(36845, 37869), REL::Relocate(0x71, 0x71),
			[](auto* param_1) {
				// 36845
				// 0x140601eb1 - 0x140601E40 = 0x71

				//AE: (99% correct, seems to match the function)
				// FUN_140629d00 (48 83 ec 58)
				// 0x140629d71 - 0x140629d00 = 0x71
				float result = GetEyeHeight_140601E40(param_1);
				float Alter = affect_by_scale(param_1, result);
				log::info("(23) GetEyeHeight_140601E40 Hooked");
				return Alter;
            }
        );*/
	}
}

