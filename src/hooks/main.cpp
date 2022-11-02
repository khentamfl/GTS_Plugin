#include "hooks/main.hpp"
#include "util.hpp"
#include "events.hpp"
#include "data/time.hpp"
#include "scale/scale.hpp"
#include "data/plugin.hpp"

using namespace RE;
using namespace SKSE;
using namespace Gts;

namespace Hooks
{
	void Hook_MainUpdate::Hook(Trampoline& trampoline)
	{
		REL::Relocation<uintptr_t> hook{REL::RelocationID(35551, 36544)};
		logger::info("Gts applying Main Update Hook at {}", hook.address());
		_Update = trampoline.write_call<5>(hook.address() + RELOCATION_OFFSET(0x11F, 0x160), Update);
	}

	void Hook_MainUpdate::Update(RE::Main* a_this, float a2)
	{
		_Update(a_this, a2);
		Time::GetSingleton().Update();
		
		static std::atomic_bool started = std::atomic_bool(false);
		Plugin::SetOnMainThread(true);
		if (Plugin::Enabled()) {
			log::info("Enabled Plugin");
			if (Plugin::InGame()) {
				// We are not loading or in the mainmenu
				auto player_char = RE::PlayerCharacter::GetSingleton();
				log::info("Player Scale is: {}, Player Max Size is: {}", get_target_scale(player_char), get_max_scale(player_char));
				if (player_char) {
					if (player_char->Is3DLoaded()) {
						// Player is loaded
						log::info("Player is Loaded");
						auto ui = RE::UI::GetSingleton();
						if (!ui->GameIsPaused()) {
							// Not paused
							if (started.exchange(true)) {
								// Not first updated
								EventDispatcher::DoUpdate();
								log::info("Doing Update");
							} else {
								// First update this load
								EventDispatcher::DoStart();
								log::info("Doing Start");
							}
						}
					}
				}
			} else {
				// Loading or in main menu
				started.store(false);
			}
		}
		Plugin::SetOnMainThread(false);
		log::info("Set On Main Thread = False");
	}
}
