#include "hooks/playerCharacter.hpp"
#include "data/runtime.hpp"
#include "data/persistent.hpp"
#include "data/plugin.hpp"
#include "util.hpp"
#include "events.hpp"

using namespace RE;
using namespace Gts;

namespace Hooks
{
	// BGSImpactManager
	void Hook_PlayerCharacter::Hook() {
		logger::info("Hooking PlayerCharacter");
		REL::Relocation<std::uintptr_t> Vtbl{ RE::VTABLE_PlayerCharacter[0] };
		_HandleHealthDamage = Vtbl.write_vfunc(REL::Relocate(0x104, 0x104, 0x106), HandleHealthDamage);
		_AddPerk = Vtbl.write_vfunc(REL::Relocate(0x0FB, 0x0FB, 0x0FD), AddPerk);
		_RemovePerk = Vtbl.write_vfunc(REL::Relocate(0x0FC, 0x0FC, 0x0FE), RemovePerk);
	}

	void Hook_PlayerCharacter::HandleHealthDamage(PlayerCharacter* a_this, Actor* a_attacker, float a_damage) {
		//log::info("PlayerCharacter::Update");
		if (a_attacker) {
			auto player = PlayerCharacter::GetSingleton();
			if (Runtime::HasPerk(player, "SizeReserveAug")) { // Size Reserve Augmentation
				auto Cache = Persistent::GetSingleton().GetData(player);
				if (Cache) {
					Cache->SizeReserve += -a_damage/3000;
				}
				log::info("  - Attacker: {}, damage: {}", a_attacker->GetDisplayFullName(), a_damage);
				a_damage *= 0.05;
				log::info("Decreasing damage to {}", a_damage);
			}
		}
		//log::info("  - Damage: {}", a_damage);
		_HandleHealthDamage(a_this, a_attacker, a_damage);
	}

	void Hook_PlayerCharacter::AddPerk(PlayerCharacter* a_this, BGSPerk* a_perk, std::uint32_t a_rank) {
		_AddPerk(a_this, a_perk, a_rank);
		AddPerk evt = AddPerk {
			.actor = a_this,
			.perk = a_perk,
			.rank = a_rank,
		};
		EventDispatcher::DoPerkAdded(evt);
	}

	void Hook_PlayerCharacter::RemovePerk(PlayerCharacter* a_this, BGSPerk* a_perk, std::uint32_t a_rank) {
		RemovePerk evt = RemovePerk {
			.actor = a_this,
			.perk = a_perk,
		};
		EventDispatcher::DoPerkRemoved(evt);
		_RemovePerk(a_this, a_perk);
	}
}
