#include "hooks/main.h"
#include "util.h"

#include "managers/contact.h"

using namespace RE;
using namespace SKSE;
using namespace Gts;

namespace Hooks
{
	void Hook_Havok::Hook(Trampoline& trampoline)
	{
		REL::Relocation<uintptr_t> hook{RELOCATION_ID(38112, 39068)};
		logger::info("Gts applying Main Update Hook at {}", hook.address());
		_ProcessHavokHitJobs = trampoline.write_call<5>(hook.address() + REL::Relocate(0x104, 0xFC), ProcessHavokHitJobs);
	}

	void Hook_Havok::ProcessHavokHitJobs(void* a1);
	{
		_ProcessHavokHitJobs(a1);

		ContactManager::GetSingleton().Update();
	}
}
