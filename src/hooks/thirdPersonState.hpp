#pragma once
#include "hooks/hooks.hpp"

using namespace RE;
using namespace SKSE;

namespace Hooks
{
	class Hook_ThirdPersonState
	{
		public:
			static void Hook();
		private:

			static void Begin(ThirdPersonState* a_this);
			static inline REL::Relocation<decltype(Begin)> _Begin;

			static void End(ThirdPersonState* a_this);
			static inline REL::Relocation<decltype(End)> _End;


			static void Update(ThirdPersonState* a_this, BSTSmartPointer<TESCameraState>& a_nextState);
			static inline REL::Relocation<decltype(Update)> _Update;

			static void GetRotation(ThirdPersonState* a_this, NiQuaternion& a_rotation);
			static inline REL::Relocation<decltype(GetRotation)> _GetRotation;

			static void GetTranslation(ThirdPersonState* a_this, NiPoint3& a_translation);
			static inline REL::Relocation<decltype(GetTranslation)> _GetTranslation;

			static void UpdateRotation(ThirdPersonState* a_this);
			static inline REL::Relocation<decltype(UpdateRotation)> _UpdateRotation;
	};
}
