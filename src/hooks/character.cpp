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
		auto index = Config::GetSingleton().GetExperiment();
		logger::info("  - Hooking table {}", index);
		REL::Relocation<std::uintptr_t> ActorVtbl{ Character::VTABLE[index] };

		_Update = ActorVtbl.write_vfunc(0xAD, Update);
		_UpdateAnimation = ActorVtbl.write_vfunc(0x7D, UpdateAnimation);
	}

	void Hook_Character::Update(RE::Character* a_this, float a_delta) {
		float anim_speed = 1.0;
		logger::info("Charcter Update");
		if (Gts::GtsManager::GetSingleton().enabled) {
			auto saved_data = Gts::Persistent::GetSingleton().GetData(a_this);
			if (saved_data) {
				if (saved_data->anim_speed > 0.0) {
					anim_speed = saved_data->anim_speed;
				}
			}
		}
		Update(a_this, a_delta * anim_speed);
	}

	void Hook_Character::UpdateAnimation(RE::Character* a_this, float a_delta) {
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
