// Holds RE and dummy classes
#include "colliders/allraycollector.hpp"

namespace RE {
  void hkpAllRayHitCollector::AddRayHit(const hkpCdBody& a_body, const hkpShapeRayCastCollectorOutput& a_hitInfo) {
    REL::Relocation<std::uintptr_t> vtable{VTABLE_hkpAllRayHitCollector[0]};
		const auto a_idx = 0x01;
		const auto addr = vtable.address() + (sizeof(void *) * a_idx);
		const auto result = *reinterpret_cast<std::uintptr_t *>(addr);
		REL::Relocation<void(hkpRayHitCollector*)> func(result);
		func(this);
  }

  hkpAllRayHitCollector::~hkpAllRayHitCollector() {
    REL::Relocation<std::uintptr_t> vtable{VTABLE_hkpAllRayHitCollector[0]};
		const auto a_idx = 0x00;
		const auto addr = vtable.address() + (sizeof(void *) * a_idx);
		const auto result = *reinterpret_cast<std::uintptr_t *>(addr);
		REL::Relocation<void(hkpRayHitCollector*)> func(result);
		func(this);
  }

  hkpAllRayHitCollector* hkpAllRayHitCollector::Create() {
    std::size_t a_size = sizeof(hkpAllRayHitCollector);
    std::uintptr_t a_vtbl = hkpAllRayHitCollector::VTABLE[0].address();
    log::info("Allocating {}", a_size);
    auto memory = malloc(a_size);
    std::memset(memory, 0, a_size);
    log::info("Hacking Vtable");
    reinterpret_cast<std::uintptr_t*>(memory)[0] = a_vtbl;
    auto result =  static_cast<hkpAllRayHitCollector*>(memory);
    log::info("Resetting data");
    result->Reset();
    return result;
  }

  std::unique_ptr<hkpAllRayHitCollector> hkpAllRayHitCollector::CreatePtr() {
    return std::unique_ptr<hkpAllRayHitCollector>(hkpAllRayHitCollector::Create());
  }
}
