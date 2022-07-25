#include "hooks/PlayerCharacter.h"
#include "util.h"
#include "managers/GtsManager.h"
#include "data/persistent.h"

using namespace RE;
using namespace SKSE;
using namespace Gts;

namespace Hooks
{
	void Hook_Player::Hook() {
		logger::info("Hooking PlayerCharacter");
		REL::Relocation<std::uintptr_t> PlayerCharacterVtbl{ RE::VTABLE_PlayerCharacter[0] };

		_UpdateAnimation = PlayerCharacterVtbl.write_vfunc(0x7D, UpdateAnimation);

		_GetRunSpeed = PlayerCharacterVtbl.write_vfunc(REL::Relocate(0x0EB, 0x0EB, 0x0ED), GetRunSpeed);
		_GetJogSpeed = PlayerCharacterVtbl.write_vfunc(REL::Relocate(0x0EC, 0x0EC, 0x0EE), GetJogSpeed);
		_GetFastWalkSpeed = PlayerCharacterVtbl.write_vfunc(REL::Relocate(0x0ED, 0x0ED, 0x0EF), GetFastWalkSpeed);
		_GetWalkSpeed = PlayerCharacterVtbl.write_vfunc(REL::Relocate(0x0EE, 0x0EE, 0x0F0), GetWalkSpeed);
	}

	void Hook_Player::UpdateAnimation(RE::PlayerCharacter* a_this, float a_delta) {
		float anim_speed = 1.0;
		if (Gts::GtsManager::GetSingleton().enabled) {
			auto saved_data = Gts::Persistent::GetSingleton().GetActorData(a_this);
			if (saved_data) {
				if (saved_data->anim_speed > 0.0) {
					anim_speed = saved_data->anim_speed;
				}
			}
		}
		_UpdateAnimation(a_this, a_delta * anim_speed);
	}

	float Hook_Player::GetRunSpeed(RE::Character* a_this) {
		float value = _GetRunSpeed(a_this);
		log::info("{} GetRunSpeed {}", actor_name(a_this), value);
		return value;
	}

	float Hook_Player::GetJogSpeed(RE::Character* a_this) {
		float value = _GetJogSpeed(a_this);
		log::info("{} GetJogSpeed {}", actor_name(a_this), value);
		return value;
	}

	float Hook_Player::GetFastWalkSpeed(RE::Character* a_this) {
		float value = _GetFastWalkSpeed(a_this);
		log::info("{} GetFastWalkSpeed {}", actor_name(a_this), value);
		return value;
	}

	float Hook_Player::GetWalkSpeed(RE::Character* a_this) {
		float value = _GetWalkSpeed(a_this);
		log::info("{} GetWalkSpeed {}", actor_name(a_this), value);
		return value;
	}
}
