#include "hooks/sink.hpp"
#include "scale/scale.hpp"

using namespace RE;
using namespace SKSE;

namespace Hooks {

void Hook_Sinking::Hook(Trampoline& trampoline) {

    static FunctionHook<float(Actor* a_this, long* unknown, TESObjectCELL* a_cell, float z_pos)> GetSubmergeLevel(
      REL::RelocationID(38850, 39887),
      REL::Offset(0x30),
        [](auto* a_this, auto* unknown, auto* cell, float z_pos){
          log::info("GetSubmergeLevel");
          if (a_this) {
            log::info("  - Actor: {}", a_this->GetDisplayFullName());
          }
          if (cell) {
            log::info("  - In a cell");
          }
          if (unknown) {
            log::info("Unknown: {}", GetRawName(reinterpret_cast<void*>(unknown)));
          }
          float result = GetSubmergeLevel(a_this, unknown, cell, z_pos);
          log::info("  - Submerged level: {}", result);
          return result;
      }
    );

	}
}
