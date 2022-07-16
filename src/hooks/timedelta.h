#pragma once
using namespace RE;
using namespace SKSE;

namespace Hooks
{
	class Hook_TimeDelta
	{
		public:
			static void Hook();
		private:

			static void PollChannelUpdateImpl(RE::BSAnimationGraphChannel* a_this, std::uint32_t poll);
			static inline REL::Relocation<decltype(PollChannelUpdateImpl)> _PollChannelUpdateImpl;
	};
}
