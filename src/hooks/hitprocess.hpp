#pragma once
#include "hooks/hooks.hpp"

using namespace RE;
using namespace SKSE;

namespace RE {

  static inline void ProcessHitEvent(RE::Actor* a_target, RE::HitData* a_hitData)
  {
      using func_t = decltype(&ProcessHitEvent);
      REL::Relocation<func_t> func{ REL::RelocationID(37633, 38586) };
      func(a_target, a_hitData);
  }

  static inline void PopulateHitData(HitData* a_this, Actor* a_aggressor, Actor* a_target, InventoryEntryData* a_weapon, bool a_bIsLeftHand = false)
	{
		using func_t = decltype(&HitData::Populate);
		REL::Relocation<func_t> func(RELOCATION_ID(42832, 44001));
		return func(a_this, a_aggressor, a_target, a_weapon, a_bIsLeftHand);
	}

  static inline HitData* HitDataCtor() {
			using func_t = decltype(&HitData::Ctor);
			REL::Relocation<func_t> func{ RELOCATION_ID(42826, 43995) };  // 742450, 76E8F0
			return func(this);
	}

  static inline HitData* CreateHitData(Actor* a_aggressor, Actor* a_target, InventoryEntryData* a_weapon, bool a_bIsLeftHand = false)
	{
		auto hitData = malloc<HitData>();
		if (hitData) {
			HitDataCtor(hitData);
			PopulateHitData(hitData, a_aggressor, a_target, a_weapon, a_bIsLeftHand);
		}

		return hitData;
	}
}
