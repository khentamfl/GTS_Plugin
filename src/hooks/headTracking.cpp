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
			REL::RelocationID(37129, 37129), REL::Relocate(0x24, 0x24),
			[](auto* param_1) {
				// 37129
				// 0x140615054 - 0x140615030 = 0x24
				// FUN_140615030

				// AE:
				// FUN_1407ea630 ? (48 89 5c 24 08)
				// FUN_140601E40 ? (48 89 5c 24 08)
				// FUN_14078e130 ? (48 89 5c 24 08)
				// FUN_14078e060....
				// FUN_1404feff0....
				// FUN_14022f300....
				float result = Alter_Headtracking(param_1);
				float Alter = affect_by_scale(param_1, result);
				//log::info("(20) Alter_Headtracking Hooked");
				return Alter;
            }
        );

		static CallHook<float(TESObjectREFR* param_1)>GetEyeHeight_140601E40(  // Get Eye Height, rarely called
			REL::RelocationID(36845, 36845), REL::Relocate(0x71, 0x71),
			[](auto* param_1) {
				// 36845
				// 0x140601eb1 - 0x140601E40 = 0x71

				//AE: (99% correct, seems to match the function)
				// FUN_140629d00 (48 83 ec 58)
				// 0x140629d71 - 0x140629d00 = 
				float result = GetEyeHeight_140601E40(param_1);
				float Alter = affect_by_scale(param_1, result);
				log::info("(23) GetEyeHeight_140601E40 Hooked");
				return Alter;
            }
        );
	}
}
