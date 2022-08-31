#include "hooks/main.hpp"
#include "util.hpp"

#include "managers/GtsManager.hpp"
#include "managers/collider.hpp"
#include "magic/magic.hpp"
#include "managers/ddraw.hpp"
#include "managers/camera.hpp"

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
		auto& manager = GtsManager::GetSingleton();
		auto& magic = MagicManager::GetSingleton();
		auto& collider = ColliderManager::GetSingleton();
		auto& ddraw = DDraw::GetSingleton();
		auto& camera = CameraManager::GetSingleton();
		activate_mainthread_mode();
		manager.poll();
		magic.Update();
		collider.Update();
		ddraw.Update();
		camera.Update();
		deactivate_mainthread_mode();
	}
}
