#pragma once
#include "hooks/hooks.hpp"

using namespace RE;
using namespace SKSE;

namespace Hooks
{
	class Hook_PlayerCamera
	{
		public:
			static void Hook();
		private:

			static void SetCameraRoot(TESCamera* a_this, NiNode a_root);
			static inline REL::Relocation<decltype(SetCameraRoot)> _SetCameraRoot;

			static void Update(TESCamera* a_this);
			static inline REL::Relocation<decltype(Update)> _Update;
	};
}
