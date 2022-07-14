#include "hooks/frame_update.h"

#include "GtsManager.h"

using namespace RE;
using namespace SKSE;

namespace Hooks
{
	void Hook_FrameUpdate::Hook(Trampoline& trampoline)
	{
		REL::Relocation<uintptr_t> hook(REL::RelocationID(35565, 36564), REL::Relocate(0x53, 0x6e));
		logger::info("Gts applying Frame Update Hook at {}", hook.address());
		_Update = trampoline.write_call<5>(hook.address(), Update);
	}

	void Hook_FrameUpdate::Update(std::int64_t unk)
	{
		_Update(unk);
		Gts::GtsManager::GetSingleton().poll();
	}
}
