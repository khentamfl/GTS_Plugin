#include "hooks/character.h"
#include "Config.h"
#include "util.h"
#include "GtsManager.h"
#include "persistent.h"


using namespace RE;
using namespace SKSE;
using namespace Gts;

namespace Hooks
{
	void Hook_Character::Hook() {
		logger::info("Hooking Character");
		REL::Relocation<std::uintptr_t> ActorVtbl{ RE::VTABLE_Character[0] };

		_Update = ActorVtbl.write_vfunc(0xAD, Update);
		_UpdateAnimation = ActorVtbl.write_vfunc(0x7D, UpdateAnimation);
	}

	void Hook_Character::Update(RE::Actor* a_this, float a_delta) {
        //if (a_delta > 1e-5) {
        //	logger::info("Charcter Update: {} by {}", actor_name(a_this), a_delta);
        //	if (Gts::GtsManager::GetSingleton().enabled) {
        //		auto saved_data = Gts::Persistent::GetSingleton().GetData(a_this);
        //		if (saved_data) {
        //			if (saved_data->anim_speed > 0.0) {
        //				a_delta *= saved_data->anim_speed;
        //			}
        //		}
        //	}
        //}
		Update(a_this, a_delta);
	}

	void Hook_Character::UpdateAnimation(RE::Actor* a_this, float a_delta) {
		log::info("Hook Character Anim: {}", actor_name(a_this));
		float anim_speed = 1.0;
		if (Gts::GtsManager::GetSingleton().enabled) {
			auto saved_data = Gts::Persistent::GetSingleton().GetData(a_this);
			if (saved_data) {
				if (saved_data->anim_speed > 0.0) {
					anim_speed = saved_data->anim_speed;
				}
			}
		}
		_UpdateAnimation(a_this, a_delta * anim_speed);
	}
}
