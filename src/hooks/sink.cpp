#include "hooks/sink.hpp"
#include "scale/scale.hpp"

using namespace RE;
using namespace SKSE;

namespace Hooks {

	void Hook_Sinking::Hook(Trampoline& trampoline) {

    static FunctionHook<float(Actor* a_this, TESObjectCELL* a_cell, float z_pos)> GetSubmergeLevel(
      RELOCATION_ID(38850, 39887),
      Offset(0x30),
        [](auto* a_this, auto* cell, float z_pos){
          float result = GetSubmergeLevel(a_this, cell, z_pos);
          log::info("GetSubmergeLevel");
          if (a_this) {
            log::info("  - Actor: {}", a_this->GetDisplayFullName());
          }
          if (cell) {
            log::info("  - In a cell");
          }
          log::info("  - Submerged level: {}", result);
          return result;
      }
    );

	}
}
