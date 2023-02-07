#include "hooks/main.hpp"
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
		logger::info("Gts applying Main Update Hook at {:X}", hook.address());
		_Update = trampoline.write_call<5>(hook.address() + RELOCATION_OFFSET(0x11F, 0x160), Update);

		if (REL::Module::IsSE()) {
			//auto offsetHelper = REL::IDDatabase::Offset2ID();
			//log::info("OFFSET 01: {}", offsetHelper(0x14067bfa0));
			//log::info("OFFSET 02: {}", offsetHelper(0x14067C659));

			//REL::Relocation<uintptr_t> unknown_hook(REL::Offset(0x14067C659));
			REL::Relocation<uintptr_t> unknown_hook{REL::RelocationID(38831, 38831)};
			REL::Relocation<uintptr_t> unknown_hook2{REL::RelocationID(36343, 36343)};
			logger::info("Applying experimental hook: {:X}", unknown_hook.address());
			logger::info("Applying experimental hook2: {:X}", unknown_hook2.address());
			_UnknownMaybeScale = trampoline.write_call<5>(unknown_hook.address() + RELOCATION_OFFSET(0x6B9,0x6B9), UnknownMaybeScale);
			_UnknownMaybeScale2 = trampoline.write_call<5>(unknown_hook2.address() + RELOCATION_OFFSET(0x6B9,0x6B9), UnknownMaybeScale2);
			logger::info("  - Applied experimental hook");
		}
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
				if (Plugin::Ready()) {
					// Player loaded and not paused
					if (started.exchange(true)) {
						// Not first updated
						EventDispatcher::DoUpdate();
					} else {
						// First update this load
						EventDispatcher::DoStart();
					}
				}
			} else {
				// Loading or in main menu
				started.store(false);
			}
		}
		Plugin::SetOnMainThread(false);

		if (Config::GetSingleton().GetDebug().ShouldProfile()) {
			static Timer timer = Timer(5.0);
			if (timer.ShouldRun()) {
				EventDispatcher::ReportProfilers();
			}
		}
	}

	void Hook_MainUpdate::UnknownMaybeScale(UnknownMaybeScaleObject* unknown_a, float a1) {
		log::info("UnknownMaybeScale");
		log::info("unknown_a: {}", GetRawName(unknown_a));
		log::info("unknown_b: {}", a1);
		//log::info("scale: {}", a2);

		// _UnknownMaybeScale(unknown_a, a1, a2);
	}
	void Hook_MainUpdate::UnknownMaybeScale2(Actor* actor, float scale) {
		log::info("UnknownMaybeScale2");
		log::info("Actor: {}", actor->GetDisplayFullName());
		log::info("Scale: {}", scale);
		//log::info("scale: {}", a2);

		// _UnknownMaybeScale(unknown_a, a1, a2);
	}
}
