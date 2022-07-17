#include "hooks/character.h"
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

		_UpdateNonRenderSafe = ActorVtbl.write_vfunc(0xB1, UpdateNonRenderSafe);
	}

	void Hook_Character::UpdateNonRenderSafe(RE::Character* a_this, float a_delta) {
		if (a_delta > 1e-5) {
			if (Gts::GtsManager::GetSingleton().enabled) {
				auto saved_data = Gts::Persistent::GetSingleton().GetActorData(a_this);
				if (saved_data) {
					if (saved_data->anim_speed > 0.0) {
						log::info("Adjusting anim speed for: {} to {}", actor_name(a_this), saved_data->anim_speed);
						a_delta *= saved_data->anim_speed;
					}
				}
			}
			log::info("Hook Character UpdateNonRenderSafe: {} by {}", actor_name(a_this), a_delta);
		}
		_UpdateNonRenderSafe(a_this, a_delta);
	}
}
