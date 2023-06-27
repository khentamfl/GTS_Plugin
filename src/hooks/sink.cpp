#include "hooks/sink.hpp"
#include "scale/scale.hpp"

using namespace RE;
using namespace SKSE;

namespace Hooks {

	void Hook_Sinking::Hook(Trampoline& trampoline) {

		static FunctionHook<float(TESObjectREFR* a_this, float z_pos, TESObjectCELL* a_cell)> GetSubmergeLevel(
			RELOCATION_ID(36452, 37448),
			[](auto* a_this, float z_pos, auto* a_cell){
  			float result = GetSubmergeLevel(a_this, z_pos, a_cell);
  			Actor* actor = skyrim_cast<Actor*>(a_this);

  			if (actor) {
          float scale = get_visual_scale(actor);
  				result/=scale;
  			}
  			return result;
		});
	}
}
