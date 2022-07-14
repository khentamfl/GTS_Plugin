#include "hooks/PlayerCharacter.h"
#include "util.h"
#include "GtsManager.h"
#include "persistent.h"

using namespace RE;
using namespace SKSE;
using namespace Gts;

namespace Hooks
{
	void Hook_Player::Hook() {
		logger::info("Hooking PlayerCharacter");
		REL::Relocation<std::uintptr_t> PlayerCharacterVtbl{ RE::VTABLE_PlayerCharacter[0] };

		_UpdateAnimation = PlayerCharacterVtbl.write_vfunc(0x7D, UpdateAnimation);
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
}
