#include "hooks/character.hpp"
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
	void Hook_Character::Hook() {
		logger::info("Hooking Character");
		REL::Relocation<std::uintptr_t> Vtbl{ RE::VTABLE_Character[0] };
		_HandleHealthDamage = Vtbl.write_vfunc(REL::Relocate(0x104, 0x104, 0x106), HandleHealthDamage);
		_AddPerk = Vtbl.write_vfunc(REL::Relocate(0x0FB, 0x0FB, 0x0FD), AddPerk);
		_RemovePerk = Vtbl.write_vfunc(REL::Relocate(0x0FC, 0x0FC, 0x0FE), RemovePerk);
		_Move = Vtbl.write_vfunc(REL::Relocate(0x0C8, 0x0C8, 0x0CA), Move);

		REL::Relocation<std::uintptr_t> Vtbl5{ RE::VTABLE_Character[5] };
		_GetActorValue = Vtbl5.write_vfunc(0x01, GetActorValue);
		_GetBaseActorValue = Vtbl5.write_vfunc(0x03, GetBaseActorValue);
	}

	void Hook_Character::HandleHealthDamage(Character* a_this, Character* a_attacker, float a_damage) {
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

	void Hook_Character::AddPerk(Character* a_this, BGSPerk* a_perk, std::uint32_t a_rank) {
		_AddPerk(a_this, a_perk, a_rank);
		AddPerkEvent evt = AddPerkEvent {
			.actor = a_this,
			.perk = a_perk,
			.rank = a_rank,
		};
		EventDispatcher::DoAddPerk(evt);
	}

	void Hook_Character::RemovePerk(Character* a_this, BGSPerk* a_perk) {
		RemovePerkEvent evt = RemovePerkEvent {
			.actor = a_this,
			.perk = a_perk,
		};
		EventDispatcher::DoRemovePerk(evt);
		_RemovePerk(a_this, a_perk);
	}

	float Hook_Character::GetActorValue(ActorValueOwner* a_owner, ActorValue a_akValue) { // Override Carry Weight and Damage
		if (Plugin::InGame()) {
			Character* a_this = skyrim_cast<Character*>(a_owner);
			if (a_this) {
				float actual_value = _GetActorValue(a_owner, a_akValue);
				float bonus = 1.0;
				auto& attributes = AttributeManager::GetSingleton();
				if (a_akValue == ActorValue::kCarryWeight) {
					bonus = attributes.GetAttributeBonus(a_this, 2.0);
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

	float Hook_Character::GetBaseActorValue(ActorValueOwner* a_owner, ActorValue a_akValue) { // Override Health
		if (Plugin::InGame()) {
			Character* a_this = skyrim_cast<Character*>(a_owner);
			float bonus = 1.0;
			if (a_this) {
				auto& attributes = AttributeManager::GetSingleton();
				if (a_akValue == ActorValue::kHealth) {
					float scale = get_visual_scale(a_this);
					float modav = a_this->GetActorValueModifier(ACTOR_VALUE_MODIFIERS::kTemporary, ActorValue::kHealth);
					float actual_value = _GetBaseActorValue(a_owner, a_akValue);
					bonus = attributes.GetAttributeBonus(a_this, 1.0);
					return actual_value*bonus + (modav*bonus -1.0);
				}
				return _GetBaseActorValue(a_owner, a_akValue);
			} else {
				return _GetBaseActorValue(a_owner, a_akValue);
			}
		} else {
			return _GetBaseActorValue(a_owner, a_akValue);
		}
	}

	void Hook_Character::Move(Character* a_this, float a_arg2, const NiPoint3& a_position) { // Override Movement Speed
		float bonus = 1.0;
		static Timer soundtimer = Timer(0.80);
		if (a_this) {
			auto& attributes = AttributeManager::GetSingleton();
			bonus = attributes.GetAttributeBonus(a_this, 3.0);
			float sizedifference = get_visual_scale(a_this)/get_visual_scale(PlayerCharacter::GetSingleton());
			float volume = a_position[1] * bonus * sizedifference / 500;
			if (soundtimer.ShouldRunFrame()) {
				Persistent::PlaySound("RumbleWalkSound", a_this, volume, 1.0);
			}
		}
		return _Move(a_this, a_arg2, a_position * bonus);
	}
}
