#pragma once
/*
 *  Hooks on various GetScale calls
 */

#include "hooks/hooks.hpp"

using namespace RE;
using namespace SKSE;

namespace Hooks
{
	class Hook_Movement
	{
		public:
			static void Hook(Trampoline& trampoline);
	};
}