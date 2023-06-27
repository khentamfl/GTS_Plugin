#include "hooks/jump.hpp"
#include "scale/scale.hpp"

using namespace RE;
using namespace SKSE;

namespace {
  Actor* FindActor(bhkCharacterController* charCont) {
    for (auto actor: find_actors()) {
      if (charCont == actor->GetCharController()) {
        return actor;
      }
    }
    return nullptr;
  }
}

namespace Hooks {

	void Hook_Jumping::Hook(Trampoline& trampoline) {


		static FunctionHook<float(bhkCharacterController* a_this)> GetFallDistance(
			REL::RelocationID(76430, 78269),
			[](auto* a_this){
    			log::info("GetFallDistance");
          float result = GetFallDistance(a_this);
          auto actor = FindActor(a_this);
          if (actor) {
            float scale = get_visual_scale(actor);
            if (scale > 1e-4) {
              result /= scale;
            }
          }
    			return result;
  			}
			);

	}
}
