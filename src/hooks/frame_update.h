#pragma once
using namespace RE;
using namespace SKSE;

namespace Hooks
{

	class Hook_FrameUpdate
	{
		public:
			static void Hook(Trampoline& trampoline);

		private:
			static void Update(std::int64_t unk);
			static inline REL::Relocation<decltype(Update)> _Update;
	};
}
