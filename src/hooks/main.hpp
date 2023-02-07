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

			class UnknownMaybeScaleObject;
			static void UnknownMaybeScale(UnknownMaybeScaleObject* unknown_a, float a1);
			static void UnknownMaybeScale2(UnknownMaybeScaleObject* unknown_a, float a1);
			static inline REL::Relocation<decltype(UnknownMaybeScale)> _UnknownMaybeScale;
	};
}
