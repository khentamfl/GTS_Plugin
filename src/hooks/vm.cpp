#include "hooks/vm.h"
#include "managers/GtsManager.h"

using namespace RE;

namespace Hooks
{
	// BGSImpactManager
	void Hook_VM::Hook() {
		logger::info("Hooking BGSImpactManager");
		REL::Relocation<std::uintptr_t> Vtbl{ RE::VTABLE_BGSImpactManager[0] };

		_ProcessEvent = Vtbl.write_vfunc(0x01, ProcessEvent);
	}

	void Hook_VM::SendEvent(IVirtualMachine* a_this, VMHandle a_handle, const BSFixedString& a_eventName, IFunctionArguments* a_args) {
		_SendEvent(a_this, a_handle, a_eventName, a_args);
		if (a_eventName.c_str() == "OnUpdate") {
			GtsManager::GetSingleton().reapply(false);
		}
	}
}
