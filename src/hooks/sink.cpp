#include "hooks/sink.hpp"
#include "scale/scale.hpp"

using namespace RE;
using namespace SKSE;

namespace Hooks {

	void Hook_Sinking::Hook(Trampoline& trampoline) {

		static FunctionHook<float(Actor* a_this, std::uintptr_t unknown, TESObjectCELL* a_cell, float z_pos)> GetSubmergeLevel(
		REL::RelocationID(36451, 37447),
		REL::Offset(0x30),
		[](auto* a_this, auto unknown, auto* cell, float z_pos){
		log::info("GetSubmergeLevel");
		std::uintptr_t log_id =  *reinterpret_cast<std::uintptr_t *>(a_this);
		float scale = 1.0;
		if (a_this) {
			log::info("  - Actor:{}: {}", log_id, a_this->GetDisplayFullName());
			scale = get_visual_scale(a_this);
		}
		if (cell) {
		 	log::info("  - In a cell:{}", log_id);
		}
		if (unknown) {
			log::info("Getting unknown:{}: {}", log_id, unknown);
		}
		
		float result = GetSubmergeLevel(a_this, unknown, cell, z_pos);
		log::info("  - Submerged level:{}:: {}", log_id, result);
		return result/scale;
		}
	);
	}
}
