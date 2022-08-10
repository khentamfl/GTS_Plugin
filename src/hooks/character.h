#pragma once
#include "hooks/hooks.h"

using namespace RE;
using namespace SKSE;

namespace Hooks
{
	class Hook_Character
	{
		public:
			static void Hook();
		private:

			static void Update(RE::Character* a_this, float a_delta);
			static inline REL::Relocation<decltype(Update)> _Update;

			static float GetRunSpeed(RE::Character* a_this);
			static inline REL::Relocation<decltype(GetRunSpeed)> _GetRunSpeed;

			static float GetJogSpeed(RE::Character* a_this);
			static inline REL::Relocation<decltype(GetJogSpeed)> _GetJogSpeed;

			static float GetFastWalkSpeed(RE::Character* a_this);
			static inline REL::Relocation<decltype(GetFastWalkSpeed)> _GetFastWalkSpeed;

			static float GetWalkSpeed(RE::Character* a_this);
			static inline REL::Relocation<decltype(GetWalkSpeed)> _GetWalkSpeed;

			static void ProcessTracking(RE::Character* a_this, float a_delta, NiAVObject* a_obj3D);
			static inline REL::Relocation<decltype(ProcessTracking)> _ProcessTracking;
	};
}
