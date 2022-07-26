#include "hooks/vm.h"
#include "managers/GtsManager.h"

using namespace RE;
using namespace RE::BSScript;
using namespace Gts;

namespace Hooks
{
	// BGSImpactManager
	void Hook_VM::Hook() {
		logger::info("Hooking VirtualMachine");
		REL::Relocation<std::uintptr_t> Vtbl{ VTABLE_BSScript__IVirtualMachine[0] };
		_SendEvent = Vtbl.write_vfunc(REL::Relocate(0x24, 0x24, 0x26), SendEvent);

		REL::Relocation<std::uintptr_t> Vtbl2{ VTABLE_BSScript__Internal__VirtualMachine[0] };
		_SendEvent2 = Vtbl2.write_vfunc(REL::Relocate(0x24, 0x24, 0x26), SendEvent);
	}

	void Hook_VM::SendEvent(IVirtualMachine* a_this, VMHandle a_handle, const BSFixedString& a_eventName, IFunctionArguments* a_args) {
		_SendEvent(a_this, a_handle, a_eventName, a_args);
		if (a_eventName.c_str() == "OnUpdate") {
			logger::info("VM OnUpdate");
			GtsManager::GetSingleton().reapply(false);
		}
	}

	void Hook_VM::SendEvent2(IVirtualMachine* a_this, VMHandle a_handle, const BSFixedString& a_eventName, IFunctionArguments* a_args) {
		_SendEvent2(a_this, a_handle, a_eventName, a_args);
		if (a_eventName.c_str() == "OnUpdate") {
			logger::info("VM OnUpdate");
			GtsManager::GetSingleton().reapply(false);
		}
	}
}
