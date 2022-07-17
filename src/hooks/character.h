#pragma once
using namespace RE;
using namespace SKSE;

namespace Hooks
{
	class Hook_Character
	{
		public:
			static void Hook();
		private:

			static void UpdateNonRenderSafe(RE::Character* a_this, float a_delta);
			static inline REL::Relocation<decltype(UpdateNonRenderSafe)> _UpdateNonRenderSafe;
	};
}
