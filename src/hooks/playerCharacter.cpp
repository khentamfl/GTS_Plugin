#include "hooks/playerCharacter.hpp"
#include "managers/Attributes.hpp"
#include "data/runtime.hpp"
#include "data/persistent.hpp"
#include "data/plugin.hpp"
#include "events.hpp"
#include "scale/scale.hpp"
#include "timer.hpp"

using namespace RE;
using namespace Gts;

namespace Hooks
{
	void Hook_PlayerCharacter::Hook() {
		logger::info("Hooking PlayerCharacter");
		REL::Relocation<std::uintptr_t> Vtbl{ RE::VTABLE_PlayerCharacter[0] };
		_HandleHealthDamage = Vtbl.write_vfunc(REL::Relocate(0x104, 0x104, 0x106), HandleHealthDamage);
		_AddPerk = Vtbl.write_vfunc(REL::Relocate(0x0FB, 0x0FB, 0x0FD), AddPerk);
		_RemovePerk = Vtbl.write_vfunc(REL::Relocate(0x0FC, 0x0FC, 0x0FE), RemovePerk);
		_SetSize = Vtbl.write_vfunc(REL::Relocate(0x0D9, 0x0D9, 0x0DB), SetSize);
		_Move = Vtbl.write_vfunc(REL::Relocate(0x0C8, 0x0C8, 0x0CA), Move);

		REL::Relocation<std::uintptr_t> Vtbl5{ RE::VTABLE_PlayerCharacter[5] };
		_GetActorValue = Vtbl5.write_vfunc(0x01, GetActorValue);
		_GetPermanentActorValue = Vtbl5.write_vfunc(0x02, GetPermanentActorValue);
		_GetBaseActorValue = Vtbl5.write_vfunc(0x03, GetBaseActorValue);
	}

	void Hook_PlayerCharacter::HandleHealthDamage(PlayerCharacter* a_this, Actor* a_attacker, float a_damage) {
		if (a_attacker) {
			if (Runtime::HasPerkTeam(a_this, "SizeReserveAug")) { // Size Reserve Augmentation
				auto Cache = Persistent::GetSingleton().GetData(a_this);
				if (Cache) {
					Cache->SizeReserve += -a_damage/3000;
				}
				a_damage *= 0.05;
			}
		}
		_HandleHealthDamage(a_this, a_attacker, a_damage);
	}

	void Hook_PlayerCharacter::AddPerk(PlayerCharacter* a_this, BGSPerk* a_perk, std::uint32_t a_rank) {
		_AddPerk(a_this, a_perk, a_rank);
		AddPerkEvent evt = AddPerkEvent {
			.actor = a_this,
			.perk = a_perk,
			.rank = a_rank,
		};
		EventDispatcher::DoAddPerk(evt);
	}

	void Hook_PlayerCharacter::RemovePerk(PlayerCharacter* a_this, BGSPerk* a_perk) {
		RemovePerkEvent evt = RemovePerkEvent {
			.actor = a_this,
			.perk = a_perk,
		};
		EventDispatcher::DoRemovePerk(evt);
		_RemovePerk(a_this, a_perk);
	}

	float Hook_PlayerCharacter::GetActorValue(ActorValueOwner* a_owner, ActorValue a_akValue) {
		if (Plugin::Ready()) {
			PlayerCharacter* a_this = skyrim_cast<PlayerCharacter*>(a_owner);
			if (a_this) {
				float actual_value = _GetActorValue(a_owner, a_akValue);
				float bonus = 1.0;
				auto& attributes = AttributeManager::GetSingleton();
				//if (a_akValue == ActorValue::kHealth) {
					//bonus = attributes.GetAttributeBonus(a_this, 1.0);
					//return actual_value * bonus;
				//}
				if (a_akValue == ActorValue::kCarryWeight) {
					bonus = attributes.GetAttributeBonus(a_this, 2.0);
					return actual_value * bonus;
				}
				if (a_akValue == ActorValue::kSpeedMult) {
					bonus = 1.0//attributes.GetAttributeBonus(a_this, 3.0);
					return actual_value * bonus;
				}
				if (a_akValue == ActorValue::kAttackDamageMult) {
					bonus = attributes.GetAttributeBonus(a_this, 4.0);
					return actual_value * bonus;
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

	float Hook_PlayerCharacter::GetPermanentActorValue(ActorValueOwner* a_owner, ActorValue a_akValue) {
		if (Plugin::Ready()) {
			PlayerCharacter* a_this = skyrim_cast<PlayerCharacter*>(a_owner);
			float bonus = 1.0;
			if (a_this) {
				//auto& attributes = AttributeManager::GetSingleton();
				//if (a_akValue == ActorValue::kHealth) {
				//	float actual_value = _GetPermanentActorValue(a_owner, a_akValue);
				//	bonus = attributes.GetAttributeBonus(a_this, 1.0);
				//	return actual_value * bonus;
				//}
				return _GetPermanentActorValue(a_owner, a_akValue);
			} else {
				return _GetPermanentActorValue(a_owner, a_akValue);
			}
		} else {
			return _GetPermanentActorValue(a_owner, a_akValue);
		}
	}
	
	float Hook_PlayerCharacter::GetBaseActorValue(ActorValueOwner* a_owner, ActorValue a_akValue) {
		if (Plugin::Ready()) {
			PlayerCharacter* a_this = skyrim_cast<PlayerCharacter*>(a_owner);
			float bonus = 1.0;
			if (a_this) {
				auto& attributes = AttributeManager::GetSingleton();
				if (a_akValue == ActorValue::kHealth) {
					float actual_value = _GetPermanentActorValue(a_owner, a_akValue);
					bonus = attributes.GetAttributeBonus(a_this, 1.0);
					return actual_value * bonus;
				}
				return _GetPermanentActorValue(a_owner, a_akValue);
			} else {
				return _GetPermanentActorValue(a_owner, a_akValue);
			}
		} else {
			return _GetPermanentActorValue(a_owner, a_akValue);
		}
	}

	void Hook_PlayerCharacter::SetSize(PlayerCharacter* a_this, float a_size) {
		log::info("Set SIZE: {}", a_size);
		// _SetSize(a_this, a_size);
	}

	void Hook_PlayerCharacter::Move(PlayerCharacter* a_this, float a_arg2, const NiPoint3& a_position) {
		return _Move(a_this, a_arg*10, a_position);
	}
}
