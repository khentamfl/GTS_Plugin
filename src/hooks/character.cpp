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

		_Update = ActorVtbl.write_vfunc(0xAD, Update);
	}

	void Hook_Character::Update(RE::Character* a_this, float a_delta) {
		_Update(a_this, a_delta);
		float current_delta = *g_delta_time;
		if (current_delta> 1e-5) {
			if (Gts::GtsManager::GetSingleton().enabled) {
				auto saved_data = Gts::Persistent::GetSingleton().GetActorData(a_this);
				if (saved_data) {
					float anim_speed = saved_data->anim_speed;
					if ((anim_speed > 0.0) && (fabs(anim_speed - 1.0) > 1e-5)) {
						float adjustment = current_delta * (anim_speed - 1.0);
						if (current_delta + adjustment > 1e-5) {
							a_this->UpdateAnimation(adjustment);
						}
					}
				}
			}
		}
	}
}
