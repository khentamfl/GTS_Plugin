// Holds RE and dummy classes
#include "colliders/allraycollector.hpp"

namespace RE {
  void hkpAllRayHitCollector::AddRayHit(const hkpCdBody& a_body, const hkpShapeRayCastCollectorOutput& a_hitInfo) {
    REL::Relocation<std::uintptr_t> vtable{VTABLE_hkpAllRayHitCollector[0]};
		const auto a_idx = 0x01;
		const auto addr = vtable.address() + (sizeof(void *) * a_idx);
		const auto result = *reinterpret_cast<std::uintptr_t *>(addr);
		REL::Relocation<void(hkpAllRayHitCollector*)> func(result);
		func(this);
  }

  hkpAllRayHitCollector::~hkpAllRayHitCollector() {
    REL::Relocation<std::uintptr_t> vtable{VTABLE_hkpAllRayHitCollector[0]};
		const auto a_idx = 0x00;
		const auto addr = vtable.address() + (sizeof(void *) * a_idx);
		const auto result = *reinterpret_cast<std::uintptr_t *>(addr);
		REL::Relocation<void(hkpAllRayHitCollector*)> func(result);
		func(this);
  }
}
