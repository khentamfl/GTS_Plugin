#pragma once
#include "hooks/hooks.hpp"

using namespace RE;
using namespace SKSE;

namespace Hooks
{
	class Hook_Player
	{
		public:
			static void Hook();
		private:

			static void UpdateAnimation(RE::PlayerCharacter* a_this, float a_delta);
			static inline REL::Relocation<decltype(UpdateAnimation)> _UpdateAnimation;

			static float GetRunSpeed(RE::Character* a_this);
			static inline REL::Relocation<decltype(GetRunSpeed)> _GetRunSpeed;

			static float GetJogSpeed(RE::Character* a_this);
			static inline REL::Relocation<decltype(GetJogSpeed)> _GetJogSpeed;

			static float GetFastWalkSpeed(RE::Character* a_this);
			static inline REL::Relocation<decltype(GetFastWalkSpeed)> _GetFastWalkSpeed;

			static float GetWalkSpeed(RE::Character* a_this);
			static inline REL::Relocation<decltype(GetWalkSpeed)> _GetWalkSpeed;
	};
}
