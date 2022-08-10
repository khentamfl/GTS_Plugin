#pragma once
#include "hooks/hooks.h"

using namespace RE;
using namespace SKSE;

namespace Hooks
{

	class Hook_Havok
	{
		public:
			static void Hook(Trampoline& trampoline);

		private:
			enum class CollisionFilterComparisonResult : uint8_t
			{
				Continue,  // Do not affect whether the two objects should collide
				Collide,   // Force the two objects to collide
				Ignore,    // Force the two objects to not collide
			};

			CollisionFilterComparisonResult CompareFilterInfo(bhkCollisionFilter* a_collisionFilter, uint32_t a_filterInfoA, uint32_t a_filterInfoB);

			static void ProcessHavokHitJobs(void* a1);
			static inline REL::Relocation<decltype(ProcessHavokHitJobs)> _ProcessHavokHitJobs;

			static bool bhkCollisionFilter_CompareFilterInfo1(RE::bhkCollisionFilter* a_this, uint32_t a_filterInfoA, uint32_t a_filterInfoB);
			static bool bhkCollisionFilter_CompareFilterInfo2(RE::bhkCollisionFilter* a_this, uint32_t a_filterInfoA, uint32_t a_filterInfoB);
			static bool bhkCollisionFilter_CompareFilterInfo3(RE::bhkCollisionFilter* a_this, uint32_t a_filterInfoA, uint32_t a_filterInfoB);
			static bool bhkCollisionFilter_CompareFilterInfo4(RE::bhkCollisionFilter* a_this, uint32_t a_filterInfoA, uint32_t a_filterInfoB);
			static bool bhkCollisionFilter_CompareFilterInfo5(RE::bhkCollisionFilter* a_this, uint32_t a_filterInfoA, uint32_t a_filterInfoB);
			static bool bhkCollisionFilter_CompareFilterInfo6(RE::bhkCollisionFilter* a_this, uint32_t a_filterInfoA, uint32_t a_filterInfoB);
			static bool bhkCollisionFilter_CompareFilterInfo7(RE::bhkCollisionFilter* a_this, uint32_t a_filterInfoA, uint32_t a_filterInfoB);
			static inline REL::Relocation<decltype(bhkCollisionFilter_CompareFilterInfo1)> _bhkCollisionFilter_CompareFilterInfo1;
			static inline REL::Relocation<decltype(bhkCollisionFilter_CompareFilterInfo2)> _bhkCollisionFilter_CompareFilterInfo2;
			static inline REL::Relocation<decltype(bhkCollisionFilter_CompareFilterInfo3)> _bhkCollisionFilter_CompareFilterInfo3;
			static inline REL::Relocation<decltype(bhkCollisionFilter_CompareFilterInfo4)> _bhkCollisionFilter_CompareFilterInfo4;
			static inline REL::Relocation<decltype(bhkCollisionFilter_CompareFilterInfo5)> _bhkCollisionFilter_CompareFilterInfo5;
			static inline REL::Relocation<decltype(bhkCollisionFilter_CompareFilterInfo6)> _bhkCollisionFilter_CompareFilterInfo6;
			static inline REL::Relocation<decltype(bhkCollisionFilter_CompareFilterInfo7)> _bhkCollisionFilter_CompareFilterInfo7;

	};
}
