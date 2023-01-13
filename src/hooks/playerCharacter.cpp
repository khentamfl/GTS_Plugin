#include "hooks/playerCharacter.hpp"
#include "data/runtime.hpp"
#include "data/persistent.hpp"
#include "data/plugin.hpp"
#include "util.hpp"

using namespace RE;
using namespace Gts;

namespace Hooks
{
	// BGSImpactManager
	void Hook_PlayerCharacter::Hook() {
		logger::info("Hooking PlayerCharacter");
		REL::Relocation<std::uintptr_t> Vtbl{ RE::VTABLE_PlayerCharacter[0] };
		_HandleHealthDamage = Vtbl.write_vfunc(REL::Relocate(0x104, 0x106, 0x26), HandleHealthDamage);
	}

	void Hook_PlayerCharacter::HandleHealthDamage(PlayerCharacter* a_this, Actor* a_attacker, float a_damage) {
		log::info("PlayerCharacter::Update");
		if (a_attacker) {
			auto player = PlayerCharacter::GetSingleton();
			if (Runtime::HasPerk(player, "SizeReserveAug")) { // Size Reserve Augmentation
				auto Cache = Persistent::GetSingleton().GetData(player);
				if (Cache) {
					Cache->SizeReserve += -a_damage/3000;
				}
				log::info("  - Attacker: {}", a_attacker->GetDisplayFullName());
			}
		}
		//log::info("  - Damage: {}", a_damage);
		_HandleHealthDamage(a_this, a_attacker, a_damage);
	}
}
