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

		_Update = ActorVtbl.write_vfunc(REL::Relocate(0xAD, 0x0AD, 0x0AF), Update);
	}

	// NPC Characters don't call UpdateAnimation like the PlayerCharacter
	// does. I was unable to find where it is updated (maybed in AI)
	// So this is an alternative hack
	// It works by either adding more animation or undoing animations
	// to that which skyrim does naturally
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
