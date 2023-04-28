#include "hooks/havok.hpp"
#include "events.hpp"

#include "managers/contact.hpp"

using namespace RE;
using namespace SKSE;
using namespace Gts;

namespace Hooks
{
	void Hook_Havok::Hook(Trampoline& trampoline)
	{
		REL::Relocation<uintptr_t> hook{RELOCATION_ID(38112, 39068)};
		logger::info("Gts applying Havok Hook at {}", hook.address());
		_ProcessHavokHitJobs = trampoline.write_call<5>(hook.address() + RELOCATION_OFFSET(0x104, 0xFC), ProcessHavokHitJobs);

		REL::Relocation<uintptr_t> collisionFilterHook1{ RELOCATION_ID(76181, 78009) };  // DAF370, DEF410, hkpRigidBody
		REL::Relocation<uintptr_t> collisionFilterHook2{ RELOCATION_ID(76676, 78548) };  // DD6780, E17640, hkpCollidableCollidableFilter_isCollisionEnabled
		REL::Relocation<uintptr_t> collisionFilterHook3{ RELOCATION_ID(76677, 78549) };  // DD67B0, E17670, hkpShapeCollectionFilter_numShapeKeyHitsLimitBreached
		REL::Relocation<uintptr_t> collisionFilterHook4{ RELOCATION_ID(76678, 78550) };  // DD68A0, E17760, hkpRayShapeCollectionFilter_isCollisionEnabled
		REL::Relocation<uintptr_t> collisionFilterHook5{ RELOCATION_ID(76679, 78551) };  // DD6900, E177C0, hkpRayCollidableFilter_isCollisionEnabled
		REL::Relocation<uintptr_t> collisionFilterHook6{ RELOCATION_ID(76680, 78552) };  // DD6930, E177F0, hkpShapeCollectionFilter_isCollisionEnabled
		REL::Relocation<uintptr_t> collisionFilterHook7{ RELOCATION_ID(77228, 79115) };  // DEE700, E30D60, hkpCachingShapePhantom

		_bhkCollisionFilter_CompareFilterInfo1 = trampoline.write_call<5>(collisionFilterHook1.address() + RELOCATION_OFFSET(0x16F, 0x16F), bhkCollisionFilter_CompareFilterInfo1);  // DAF4DF, DEF57F
		_bhkCollisionFilter_CompareFilterInfo2 = trampoline.write_call<5>(collisionFilterHook2.address() + RELOCATION_OFFSET(0x17, 0x17), bhkCollisionFilter_CompareFilterInfo2);    // DD6797, E17657
		_bhkCollisionFilter_CompareFilterInfo3 = trampoline.write_call<5>(collisionFilterHook3.address() + RELOCATION_OFFSET(0xBC, 0xBC), bhkCollisionFilter_CompareFilterInfo3);    // DD686C, E1772C
		_bhkCollisionFilter_CompareFilterInfo4 = trampoline.write_call<5>(collisionFilterHook4.address() + RELOCATION_OFFSET(0x31, 0x31), bhkCollisionFilter_CompareFilterInfo4);    // DD68D1, E17791
		_bhkCollisionFilter_CompareFilterInfo5 = trampoline.write_call<5>(collisionFilterHook5.address() + RELOCATION_OFFSET(0x17, 0x17), bhkCollisionFilter_CompareFilterInfo5);    // DD6917, E177D7
		_bhkCollisionFilter_CompareFilterInfo6 = trampoline.write_call<5>(collisionFilterHook6.address() + RELOCATION_OFFSET(0x118, 0x118), bhkCollisionFilter_CompareFilterInfo6);  // DD6A48, E17908
		_bhkCollisionFilter_CompareFilterInfo7 = trampoline.write_call<5>(collisionFilterHook7.address() + RELOCATION_OFFSET(0x128, 0x128), bhkCollisionFilter_CompareFilterInfo7);  // DEE828, E30E88
	}

	void Hook_Havok::ProcessHavokHitJobs(void* a1)
	{
		_ProcessHavokHitJobs(a1);

		EventDispatcher::DoHavokUpdate();
	}

	Hook_Havok::CollisionFilterComparisonResult Hook_Havok::CompareFilterInfo(RE::bhkCollisionFilter* a_collisionFilter, uint32_t a_filterInfoA, uint32_t a_filterInfoB)
	{
		// COL_LAYER layerA = static_cast<COL_LAYER>(a_filterInfoA & 0x7f);
		// COL_LAYER layerB = static_cast<COL_LAYER>(a_filterInfoB & 0x7f);
		//
		// if ((layerA == COL_LAYER::kBiped || layerA == COL_LAYER::kBipedNoCC) && (layerB == COL_LAYER::kBiped || layerB == COL_LAYER::kBipedNoCC)) {
		// 	// Biped vs. biped
		// 	uint16_t groupA = a_filterInfoA >> 16;
		// 	uint16_t groupB = a_filterInfoB >> 16;
		// 	if (groupA == groupB) {
		// 		return CollisionFilterComparisonResult::Ignore;
		// 	}
		//
		// 	return CollisionFilterComparisonResult::Collide;
		// }

		return CollisionFilterComparisonResult::Continue;
	}
	bool Hook_Havok::bhkCollisionFilter_CompareFilterInfo1(RE::bhkCollisionFilter* a_this, uint32_t a_filterInfoA, uint32_t a_filterInfoB)
	{
		switch (CompareFilterInfo(a_this, a_filterInfoA, a_filterInfoB)) {
			case CollisionFilterComparisonResult::Continue:
			default:
				return _bhkCollisionFilter_CompareFilterInfo1(a_this, a_filterInfoA, a_filterInfoB);
			case CollisionFilterComparisonResult::Collide:
				return true;
			case CollisionFilterComparisonResult::Ignore:
				return false;
		}
	}

	bool Hook_Havok::bhkCollisionFilter_CompareFilterInfo2(RE::bhkCollisionFilter* a_this, uint32_t a_filterInfoA, uint32_t a_filterInfoB)
	{
		switch (CompareFilterInfo(a_this, a_filterInfoA, a_filterInfoB)) {
			case CollisionFilterComparisonResult::Continue:
			default:
				return _bhkCollisionFilter_CompareFilterInfo2(a_this, a_filterInfoA, a_filterInfoB);
			case CollisionFilterComparisonResult::Collide:
				return true;
			case CollisionFilterComparisonResult::Ignore:
				return false;
		}
	}

	bool Hook_Havok::bhkCollisionFilter_CompareFilterInfo3(RE::bhkCollisionFilter* a_this, uint32_t a_filterInfoA, uint32_t a_filterInfoB)
	{
		switch (CompareFilterInfo(a_this, a_filterInfoA, a_filterInfoB)) {
			case CollisionFilterComparisonResult::Continue:
			default:
				return _bhkCollisionFilter_CompareFilterInfo3(a_this, a_filterInfoA, a_filterInfoB);
			case CollisionFilterComparisonResult::Collide:
				return true;
			case CollisionFilterComparisonResult::Ignore:
				return false;
		}
	}

	bool Hook_Havok::bhkCollisionFilter_CompareFilterInfo4(RE::bhkCollisionFilter* a_this, uint32_t a_filterInfoA, uint32_t a_filterInfoB)
	{
		switch (CompareFilterInfo(a_this, a_filterInfoA, a_filterInfoB)) {
			case CollisionFilterComparisonResult::Continue:
			default:
				return _bhkCollisionFilter_CompareFilterInfo4(a_this, a_filterInfoA, a_filterInfoB);
			case CollisionFilterComparisonResult::Collide:
				return true;
			case CollisionFilterComparisonResult::Ignore:
				return false;
		}
	}

	bool Hook_Havok::bhkCollisionFilter_CompareFilterInfo5(RE::bhkCollisionFilter* a_this, uint32_t a_filterInfoA, uint32_t a_filterInfoB)
	{
		switch (CompareFilterInfo(a_this, a_filterInfoA, a_filterInfoB)) {
			case CollisionFilterComparisonResult::Continue:
			default:
				return _bhkCollisionFilter_CompareFilterInfo5(a_this, a_filterInfoA, a_filterInfoB);
			case CollisionFilterComparisonResult::Collide:
				return true;
			case CollisionFilterComparisonResult::Ignore:
				return false;
		}
	}

	bool Hook_Havok::bhkCollisionFilter_CompareFilterInfo6(RE::bhkCollisionFilter* a_this, uint32_t a_filterInfoA, uint32_t a_filterInfoB)
	{
		switch (CompareFilterInfo(a_this, a_filterInfoA, a_filterInfoB)) {
			case CollisionFilterComparisonResult::Continue:
			default:
				return _bhkCollisionFilter_CompareFilterInfo6(a_this, a_filterInfoA, a_filterInfoB);
			case CollisionFilterComparisonResult::Collide:
				return true;
			case CollisionFilterComparisonResult::Ignore:
				return false;
		}
	}

	bool Hook_Havok::bhkCollisionFilter_CompareFilterInfo7(RE::bhkCollisionFilter* a_this, uint32_t a_filterInfoA, uint32_t a_filterInfoB)
	{
		switch (CompareFilterInfo(a_this, a_filterInfoA, a_filterInfoB)) {
			case CollisionFilterComparisonResult::Continue:
			default:
				return _bhkCollisionFilter_CompareFilterInfo7(a_this, a_filterInfoA, a_filterInfoB);
			case CollisionFilterComparisonResult::Collide:
				return true;
			case CollisionFilterComparisonResult::Ignore:
				return false;
		}
	}
}
