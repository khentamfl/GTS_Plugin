#include "hooks/main.hpp"
#include "util.hpp"
#include "events.hpp"
#include "data/time.hpp"
#include "data/plugin.hpp"
#include "timer.hpp"
#include "Config.hpp"

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
			if (Plugin::InGame()) {
				// We are not loading or in the mainmenu
				auto player_char = RE::PlayerCharacter::GetSingleton();
				if (player_char) {
					if (player_char->Is3DLoaded()) {
						// Player is loaded
						auto ui = RE::UI::GetSingleton();
						if (!ui->GameIsPaused()) {
							// Not paused
							if (started.exchange(true)) {
								// Not first updated
								EventDispatcher::DoUpdate();
							} else {
								// First update this load
								EventDispatcher::DoStart();
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

		if (Config::GetSingleton().ShouldProfile()) {
			static Timer timer = Timer(5.0);
			if (timer.ShouldRun()) {
				EventDispatcher::ReportProfilers();
			}
		}
	}
}
