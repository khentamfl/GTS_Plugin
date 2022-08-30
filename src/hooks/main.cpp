#include "hooks/main.hpp"
#include "util.hpp"

#include "managers/GtsManager.hpp"
#include "managers/GtsQuest.hpp"
#include "managers/camera.hpp"
#include "managers/Attributes.hpp"
#include "managers/RandomGrowth.hpp"
#include "magic/magic.hpp"

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
		auto& camera = CameraManager::GetSingleton();
		auto& quest = QuestManager::GetSingleton();
		auto& attributes = AttributeManager::GetSingleton();
		auto& randomGrowth = RandomGrowth::GetSingleton();
		activate_mainthread_mode();
		manager.poll();
		magic.Update();
		camera.Update();
		quest.Update();
		attributes.Update();
		randomGrowth.Update();

		deactivate_mainthread_mode();
	}
}
