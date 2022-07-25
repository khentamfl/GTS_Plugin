#pragma once
using namespace RE;
using namespace SKSE;

using namespace RE;
using namespace RE::BSScript;

namespace Hooks
{
	class Hook_VM
	{
		public:
			static void Hook();
		private:

			static void SendEvent(IVirtualMachine* a_this, VMHandle a_handle, const BSFixedString& a_eventName, IFunctionArguments* a_args);
			static inline REL::Relocation<decltype(SendEvent)> _SendEvent;
	};
}
