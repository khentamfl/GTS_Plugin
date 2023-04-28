#pragma once
// See https://github.com/ersh1/TrueDirectionalMovement/blob/b164fb5500a72f179fa2f0001e15397094f77c73/src/Hooks.h#L378-L387
#include "hooks/hooks.hpp"

using namespace RE;
using namespace SKSE;

namespace Hooks
{

	class Hook_HeadTracking
	{
		public:
			static void Hook(Trampoline& trampoline);

		private:

			static void SetHeadtrackTarget0(RE::AIProcess* a_this, RE::Actor* a_target);
			static void SetHeadtrackTarget4A(RE::AIProcess* a_this, RE::Actor* a_target);
			static void SetHeadtrackTarget4B(RE::AIProcess* a_this, RE::Actor* a_target);
			static void SetHeadtrackTarget4C(RE::AIProcess* a_this, RE::Actor* a_target);
			static void SetHeadtrackTarget4D(RE::AIProcess* a_this, RE::Actor* a_target);
			static inline REL::Relocation<decltype(SetHeadtrackTarget0)> _SetHeadtrackTarget0;
			static inline REL::Relocation<decltype(SetHeadtrackTarget4A)> _SetHeadtrackTarget4A;
			static inline REL::Relocation<decltype(SetHeadtrackTarget4B)> _SetHeadtrackTarget4B;
			static inline REL::Relocation<decltype(SetHeadtrackTarget4C)> _SetHeadtrackTarget4C;
			static inline REL::Relocation<decltype(SetHeadtrackTarget4D)> _SetHeadtrackTarget4D;
	};
}
