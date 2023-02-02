#include "hooks/actor.hpp"
#include "data/runtime.hpp"
#include "data/persistent.hpp"
#include "data/plugin.hpp"
#include "events.hpp"

using namespace RE;
using namespace Gts;

namespace Hooks
{
	void Hook_Actor::Hook() {
		logger::info("Hooking Actor");
		REL::Relocation<std::uintptr_t> Vtbl{ RE::VTABLE_Actor[0] };
		_HandleHealthDamage = Vtbl.write_vfunc(REL::Relocate(0x104, 0x104, 0x106), HandleHealthDamage);
		_AddPerk = Vtbl.write_vfunc(REL::Relocate(0x0FB, 0x0FB, 0x0FD), AddPerk);
		_RemovePerk = Vtbl.write_vfunc(REL::Relocate(0x0FC, 0x0FC, 0x0FE), RemovePerk);

		REL::Relocation<std::uintptr_t> Vtbl5{ RE::VTABLE_Actor[6] };
		_GetActorValue = Vtbl5.write_vfunc(0x01, GetActorValue);
		_GetPermanentActorValue = Vtbl5.write_vfunc(0x02, GetPermanentActorValue);
	}

	void Hook_Actor::HandleHealthDamage(Actor* a_this, Actor* a_attacker, float a_damage) {
		if (a_attacker) {
			if (Runtime::HasPerkTeam(a_this, "SizeReserveAug")) { // Size Reserve Augmentation
				auto Cache = Persistent::GetSingleton().GetData(a_this);
				if (Cache) {
					Cache->SizeReserve += -a_damage/3000;
				}
				a_damage *= 0.05; // Decrease received damage
			}
		}
		_HandleHealthDamage(a_this, a_attacker, a_damage);  // Just reports the value, can't override it.
	}

	void Hook_Actor::AddPerk(Actor* a_this, BGSPerk* a_perk, std::uint32_t a_rank) {
		_AddPerk(a_this, a_perk, a_rank);
		AddPerkEvent evt = AddPerkEvent {
			.actor = a_this,
			.perk = a_perk,
			.rank = a_rank,
		};
		EventDispatcher::DoAddPerk(evt);
	}

	void Hook_Actor::RemovePerk(Actor* a_this, BGSPerk* a_perk) {
		RemovePerkEvent evt = RemovePerkEvent {
			.actor = a_this,
			.perk = a_perk,
		};
		EventDispatcher::DoRemovePerk(evt);
		_RemovePerk(a_this, a_perk);
	}

	float Hook_Actor::GetActorValue(ActorValueOwner* a_owner, ActorValue a_akValue) {
		if (Plugin::Ready()) {
			Actor* a_this = skyrim_cast<Actor*>(a_owner);
			if (a_this) {
				log::info("Get AV");
				float actual_value = _GetActorValue(a_owner, a_akValue);
				if (a_akValue == ActorValue::kArchery) {
					return actual_value + 100000.0;
				} else {
					return actual_value;
				}
			} else {
				return _GetActorValue(a_owner, a_akValue);
			}
		} else {
			return _GetActorValue(a_owner, a_akValue);
		}
	}

	float Hook_Actor::GetPermanentActorValue(ActorValueOwner* a_owner, ActorValue a_akValue) {
		if (Plugin::Ready()) {
			Actor* a_this = skyrim_cast<Actor*>(a_owner);
			if (a_this) {
				log::info("Get Perma AV");
				float actual_value = _GetPermanentActorValue(a_owner, a_akValue);
				return actual_value;
			} else {
				return _GetPermanentActorValue(a_owner, a_akValue);
			}
		} else {
			return _GetPermanentActorValue(a_owner, a_akValue);
		}
	}
}
