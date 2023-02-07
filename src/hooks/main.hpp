#pragma once
#include "hooks/hooks.hpp"

using namespace RE;
using namespace SKSE;

namespace Hooks
{

	class Hook_MainUpdate
	{
		public:
			static void Hook(Trampoline& trampoline);

		private:
			static void Update(RE::Main* a_this, float a2);
			static inline REL::Relocation<decltype(Update)> _Update;

			static void UnknownMaybeScale(void* unknown_a, float a2, void* unknown_b);
			static inline REL::Relocation<decltype(UnknownMaybeScale)> _UnknownMaybeScale;
	};
}
