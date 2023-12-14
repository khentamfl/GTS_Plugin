#include "hooks/damage.hpp"

using namespace RE;
using namespace SKSE;

namespace {

}

namespace Hooks
{

	void Hook_Damage::Hook(Trampoline& trampoline) {
		static FunctionHook<void(Actor* a_this, float dmg, uintptr_t maybe_hit_data, Actor* aggressor,TESObjectREFR* damageSrc)> SkyrimTakeDamage(
      RELOCATION_ID(36345, 37335),
      [](auto* a_this, auto dmg, auto maybe_hit_data,auto* aggressor,auto* damageSrc) {
        log::info("{}: Taking {} damge", a_this->GetDisplayFullName(), dmg);
        log::info("    - Reducing damage to 1.0");
        dmg = 1.0;
        SkyrimTakeDamage(a_this, dmg, maybe_hit_data, aggressor, damageSrc);
        return;
			}
    );
	}
}
