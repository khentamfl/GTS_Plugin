#pragma once
using namespace RE;
using namespace SKSE;

namespace Hooks
{

	class Hook_Havok
	{
		public:
			static void Hook(Trampoline& trampoline);

		private:
			static void ProcessHavokHitJobs(void* a1);
			static inline REL::Relocation<decltype(ProcessHavokHitJobs)> _ProcessHavokHitJobs;
	};
}
