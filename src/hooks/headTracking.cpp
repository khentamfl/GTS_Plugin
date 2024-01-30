#include "hooks/headTracking.hpp"
#include "utils/actorUtils.hpp"
#include "scale/scale.hpp"

using namespace RE;
using namespace SKSE;


namespace {
	float affect_by_scale(TESObjectREFR* ref, float original) {
		Actor* giant = skyrim_cast<Actor*>(ref);
		if (giant) {
			float scale = get_giantess_scale(giant);
			return scale;
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
				float result = GetEyeHeight_140601E40(param_1);
				float Alter = affect_by_scale(param_1, result);
				log::info("(23) GetEyeHeight_140601E40 Hooked");
				return Alter;
            }
        );
	}
}
