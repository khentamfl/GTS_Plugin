#pragma once
#include "hooks/hooks.h"

using namespace RE;
using namespace SKSE;

namespace Hooks
{
	class Hook_Actor
	{
		public:
			static void Hook();
		private:

			static void UpdateAnimation(RE::Actor* a_this, float a_delta);
			static inline REL::Relocation<decltype(UpdateAnimation)> _UpdateAnimation;
	};
}
