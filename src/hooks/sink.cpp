#include "hooks/sink.hpp"
#include "scale/scale.hpp"

using namespace RE;
using namespace SKSE;

namespace Hooks {

	void Hook_Sinking::Hook(Trampoline& trampoline) {

		static FunctionHook<float(TESObjectREFR* a_this, float z_pos, TESObjectCELL* a_cell)> GetSubmergeLevel(
			RELOCATION_ID(36452, 37448),
			[](auto* a_this, float z_pos, auto* a_cell){
  			log::info("GetSubmergeLevel");
  			std::uintptr_t log_id =  *reinterpret_cast<std::uintptr_t *>(a_this);
  			if (a_this) {
  				log::info("  - Actor:{}: {}", log_id, a_this->GetDisplayFullName());
  			}
  			if (a_cell) {
  				log::info("  - In a cell:{}", log_id);
  			}
			float result = GetSubmergeLevel(a_this, z_pos, a_cell);
			Actor* is_actor = skyrim_cast<Actor*>(a_this);
			//float scale = 1.0;
			if (is_actor) {
			 	//scale = std::clamp(get_visual_scale(is_actor), 0.10f, 9999999.0f);
				if (is_actor->formID == 0x14) {
					log::info("  - Submerged level:{}:: {}", log_id, result);
					return 0.0;
				}
			}
  			return result;
			});
	}
}
