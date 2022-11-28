#pragma once
#include "hooks/hooks.hpp"

using namespace RE;
using namespace SKSE;

namespace Hooks
{
	class Hook_CameraState
	{
		public:
			static void Hook();
		private:

			static void Update(ThirdPersonState* a_this, BSTSmartPointer<TESCameraState>& a_nextState);
			static inline REL::Relocation<decltype(Update)> _Update;

			static void GetRotation(ThirdPersonState* a_this, NiQuaternion& a_rotation);
			static inline REL::Relocation<decltype(GetRotation)> _GetRotation;

			static void GetTranslation(ThirdPersonState* a_this, NiPoint3& a_translation);
			static inline REL::Relocation<decltype(GetTranslation)> _GetTranslation;
	};
}
