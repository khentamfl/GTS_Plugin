#include "hooks/tesobjectrefr.h"
#include "util.h"
#include "GtsManager.h"
#include "persistent.h"

using namespace RE;
using namespace SKSE;
using namespace Gts;

namespace Hooks
{
	void Hook_Character::Hook() {
		logger::info("Hooking TESObjectRefr");
		REL::Relocation<std::uintptr_t> Vtbl{ RE::VTABLE_TESObjectREFR[0] };

		_UpdateAnimation = Vtbl.write_vfunc(0x7D, UpdateAnimation);
	}

	void Hook_Character::UpdateAnimation(RE::TESObjectREFR* a_this, float a_delta) {
		log::info("Hook TESObjectREFR Anim", a_this->GetDisplayFullName());
		float anim_speed = 1.0;
		if (Gts::GtsManager::GetSingleton().enabled) {
			auto saved_data = Gts::Persistent::GetSingleton().GetData(a_this);
			if (saved_data) {
				if (saved_data->anim_speed > 0.0) {
					log::info("Adjusting anim speed for: {} to {}", a_this->GetDisplayFullName(), saved_data->anim_speed);
					anim_speed = saved_data->anim_speed;
				} else {
					log::info("anim speed too low: {}", a_this->GetDisplayFullName());
				}
			} else {
				log::info("No saved data for: {}", a_this->GetDisplayFullName());
			}
		} else {
			log::info("Not enabled");
		}
		_UpdateAnimation(a_this, a_delta * anim_speed);
	}
}
