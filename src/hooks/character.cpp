#include "hooks/character.hpp"
#include "managers/hitmanager.hpp"
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
		//_ProcessTracking = (Vtbl.write_vfunc(REL::Relocate(0x122, 0x122, 0x124), ProcessTracking));

		REL::Relocation<std::uintptr_t> Vtbl5{ RE::VTABLE_Character[5] };
		_GetActorValue = Vtbl5.write_vfunc(0x01, GetActorValue);
		_GetPermanentActorValue = Vtbl5.write_vfunc(0x02, GetPermanentActorValue);
		_GetBaseActorValue = Vtbl5.write_vfunc(0x03, GetBaseActorValue);
		_SetBaseActorValue = Vtbl5.write_vfunc(0x04, SetBaseActorValue);

		REL::Relocation<std::uintptr_t> AnimVtbl{ RE::VTABLE_Character[2] };
		_NPCAnimEvents = AnimVtbl.write_vfunc(0x1, &NPCAnimEvents);
	}

	void Hook_Character::HandleHealthDamage(Character* a_this, Character* a_attacker, float a_damage) {
		if (a_attacker) {
			auto charCont = a_this->GetCharController();
			if (charCont) {
				float sizedifference = get_visual_scale(a_this)/get_visual_scale(a_attacker);
				a_this->SetGraphVariableFloat("GiantessScale", sizedifference); // Manages Stagger Resistance inside Behaviors.
			}
			float damagemult = AttributeManager::GetSingleton().GetAttributeBonus(a_attacker, ActorValue::kAttackDamageMult);
			float damage = (a_damage * damagemult) - a_damage;
			if (damage < 0) {
				DamageAV(a_this, ActorValue::kHealth, -damage); // Damage hp
			}
			if (damage > 0) {
				a_this->AsActorValueOwner()->RestoreActorValue(ACTOR_VALUE_MODIFIER::kDamage, ActorValue::kHealth, damage); // Restore hp
			}
			if (Runtime::HasPerkTeam(a_this, "SizeReserveAug")) { // Size Reserve Augmentation
				auto Cache = Persistent::GetSingleton().GetData(a_this);
				if (Cache) {
					Cache->SizeReserve += -a_damage/3000;
				}
			}
			if (damage > GetAV(a_this, ActorValue::kHealth) * 1.5) { // Overkill effect
				float attackerscale = get_visual_scale(a_attacker);
				float receiverscale = get_visual_scale(a_this);
				if (IsDragon(a_this)) {
					receiverscale *= 2.0;
				}
				float size_difference = attackerscale/receiverscale;
				if (size_difference >= 18.0) {
					HitManager::GetSingleton().Overkill(a_this, a_attacker);
				}
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
		float value = _GetActorValue(a_owner, a_akValue);
		if (Plugin::InGame()) {
			Actor* a_this = skyrim_cast<Actor*>(a_owner);
			if (a_this) {
				value = AttributeManager::AlterGetAv(a_this, a_akValue, value);
			}
		}
		return value;
	}

	float Hook_Character::GetBaseActorValue(ActorValueOwner* a_owner, ActorValue a_akValue) { // Override Health
		float value = _GetBaseActorValue(a_owner, a_akValue);
		if (Plugin::InGame()) {
			Actor* a_this = skyrim_cast<Actor*>(a_owner);
			float bonus = 1.0;
			if (a_this) {
				value = AttributeManager::AlterGetBaseAv(a_this, a_akValue, value);
			}
		}
		return value;
	}

	void Hook_Character::SetBaseActorValue(ActorValueOwner* a_owner, ActorValue a_akValue, float value) {
		if (Plugin::InGame()) {
			Actor* a_this = skyrim_cast<Actor*>(a_owner);
			if (a_this) {
				value = AttributeManager::AlterSetBaseAv(a_this, a_akValue, value);
			}
		}
		_SetBaseActorValue(a_owner, a_akValue, value);
	}

	float Hook_Character::GetPermanentActorValue(ActorValueOwner* a_owner, ActorValue a_akValue) { // Override Carry Weight and Damage
		float value = _GetPermanentActorValue(a_owner, a_akValue);
		if (Plugin::InGame()) {
			Actor* a_this = skyrim_cast<Actor*>(a_owner);
			if (a_this) {
				value = AttributeManager::AlterGetPermenantAv(a_this, a_akValue, value);
			}
		}
		return value;
	}

	void Hook_Character::Move(Character* a_this, float a_arg2, const NiPoint3& a_position) { // Override Movement Speed
		if (a_this->IsInKillMove()) {
			return _Move(a_this, a_arg2, a_position); // Do nothing in Kill moves
		}
		float bonus = AttributeManager::AlterMovementSpeed(a_this, a_position);
		return _Move(a_this, a_arg2, a_position * bonus);
	}

	void Hook_Character::ProcessTracking(Character* a_this, float a_delta, NiAVObject* a_obj3D) {
		float adjust = Runtime::GetFloat("ConversationCameraComp");
		//if (a_this) {
		//log::info("{} Is head-tracking the Player", a_this->GetDisplayFullName());
		//}
		auto player = PlayerCharacter::GetSingleton()->Get3D();
		_ProcessTracking(a_this, a_delta, player);
	}

	void Hook_Character::NPCAnimEvents(BSTEventSink<BSAnimationGraphEvent>* a_this, BSAnimationGraphEvent& a_event, BSTEventSource<BSAnimationGraphEvent>* a_src) {
		if (a_event.tag != NULL && a_event.holder != NULL) {
			Actor* actor = static_cast<Actor*>(a_this);
			if (actor) {
				EventDispatcher::DoActorAnimEvent(actor, a_event.tag, a_event.payload);
			}
		}
		return _NPCAnimEvents(a_this, a_event, a_src);
	}
}
