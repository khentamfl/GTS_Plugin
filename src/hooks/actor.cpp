#include "hooks/actor.hpp"
#include "util.hpp"
#include "managers/GtsManager.hpp"
#include "data/persistent.hpp"

using namespace RE;
using namespace SKSE;
using namespace Gts;

namespace Hooks
{
	void Hook_Actor::Hook() {
		logger::info("Hooking Actor");
		REL::Relocation<std::uintptr_t> ActorVtbl{ RE::VTABLE_Actor[0] };

		_UpdateAnimation = ActorVtbl.write_vfunc(0x7D, UpdateAnimation);
	}

	void Hook_Actor::UpdateAnimation(RE::Actor* a_this, float a_delta) {
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
