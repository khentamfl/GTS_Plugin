#include "hooks/character.h"
#include "util.h"
#include "GtsManager.h"


using namespace RE;

namespace Hooks
{
	void Hook_Character::Hook() {
		logger::info("Hooking Actor");
		REL::Relocation<std::uintptr_t> ActorVtbl{ RE::VTABLE_Character[0] };

		_UpdateAnimation = ActorVtbl.write_vfunc(0x7D, UpdateAnimation);
	}

	void Hook_Character::UpdateAnimation(RE::Actor* a_this, float a_delta) {
		log::info("Hook Character Anim: {}", actor_name(a_this));
		float anim_speed = 1.0;
		if (Gts::GtsManager::GetSingleton().enabled) {
			auto saved_data = Gts::Persistent::GetSingleton().GetActorData(a_this);
			if (saved_data) {
				if (saved_data->anim_speed > 0.0) {
					log::info("Adjusting anim speed for: {} to {}", actor_name(a_this), saved_data->anim_speed);
					anim_speed = saved_data->anim_speed;
				} else {
					log::info("anim speed too low: {}", actor_name(a_this));
				}
			} else {
				log::info("No saved data for: {}", actor_name(a_this));
			}
		} else {
			log::info("Not enabled");
		}
		_UpdateAnimation(a_this, a_delta * anim_speed);
	}
}
