#include "hooks/havok.hpp"
#include "events.hpp"
#include "data/transient.hpp"

#include "managers/contact.hpp"
#include "data/runtime.hpp"

using namespace RE;
using namespace SKSE;
using namespace Gts;

namespace {
  COL_LAYER GetCollisionLayer(const std::uint32_t& collisionFilterInfo) {
    return static_cast<COL_LAYER>(collisionFilterInfo & 0x7F);
  }
  COL_LAYER GetCollisionLayer(const hkpCollidable* collidable) {
    if (collidable) {
      return GetCollisionLayer(collidable->broadPhaseHandle.collisionFilterInfo);
    } else {
      return COL_LAYER::kUnidentified;
    }
  }
  COL_LAYER GetCollisionLayer(const hkpCollidable& collidable) {
    return GetCollisionLayer(&collidable);
  }

  std::uint32_t GetCollisionSystem(const std::uint32_t& collisionFilterInfo) {
    return collisionFilterInfo >> 16;
  }
  std::uint32_t GetCollisionSystem(const hkpCollidable* collidable) {
    if (collidable) {
      return GetCollisionSystem(collidable->broadPhaseHandle.collisionFilterInfo);
    } else {
      return 0;
    }
  }
  std::uint32_t GetCollisionSystem(const hkpCollidable& collidable) {
    return GetCollisionSystem(&collidable);
  }

  TESObjectREFR* GetTESObjectREFR(const hkpCollidable* collidable) {
    if (collidable) {
      auto type = collidable->broadPhaseHandle.type;
      if (static_cast<RE::hkpWorldObject::BroadPhaseType>(type) == hkpWorldObject::BroadPhaseType::kEntity) {
        if (collidable->ownerOffset < 0) {
          hkpRigidBody* obj = collidable->GetOwner<hkpRigidBody>();
          if (obj) {
            return obj->GetUserData();
          }
        }
      } else if (static_cast<RE::hkpWorldObject::BroadPhaseType>(type) == hkpWorldObject::BroadPhaseType::kPhantom) {
        if (collidable->ownerOffset < 0) {
          hkpPhantom* obj = collidable->GetOwner<hkpPhantom>();
          if (obj) {
            return obj->GetUserData();
          }
        }
      }
    }
    return nullptr;
  }
  TESObjectREFR* GetTESObjectREFR(const hkpCollidable& collidable) {
    return GetTESObjectREFR(&collidable);
  }

  bool IsCollisionDisabledBetween(TESObjectREFR* actor, TESObjectREFR* otherActor) {
    if (!actor) {
      return false;
    }
    if (!otherActor) {
      return false;
    }
    auto tranData = Transient::GetSingleton().GetData(actor);
    if (tranData) {
      if (tranData->disable_collision_with == otherActor) {
        return true;
      }
    }

    auto tranDataB = Transient::GetSingleton().GetData(otherActor);
    if (tranDataB) {
      if (tranDataB->disable_collision_with == actor) {
        return true;
      }
    }

    return false;
  }
}

namespace Hooks
{
	void Hook_Havok::Hook(Trampoline& trampoline)
	{
		REL::Relocation<uintptr_t> hook{RELOCATION_ID(38112, 39068)};
		logger::info("Gts applying Havok Hook at {}", hook.address());
		_ProcessHavokHitJobs = trampoline.write_call<5>(hook.address() + RELOCATION_OFFSET(0x104, 0xFC), ProcessHavokHitJobs);

    REL::Relocation<std::uintptr_t> Vtbl{ RE::VTABLE_bhkCollisionFilter[1] };
		_IsCollisionEnabled = Vtbl.write_vfunc(0x1, IsCollisionEnabled);
	}

	void Hook_Havok::ProcessHavokHitJobs(void* a1)
	{
		_ProcessHavokHitJobs(a1);

		EventDispatcher::DoHavokUpdate();
	}

  // Credit: FlyingParticle for code on getting the TESObjectREFR
  //         maxsu. for IsCollisionEnabled idea
  bool* Hook_Havok::IsCollisionEnabled(hkpCollidableCollidableFilter* a_this, bool* a_result, const hkpCollidable* a_collidableA, const hkpCollidable* a_collidableB) {
    a_result = _IsCollisionEnabled(a_this, a_result, a_collidableA, a_collidableB);
    if (*a_result) {
      auto colLayerA = GetCollisionLayer(a_collidableA);
      auto colLayerB = GetCollisionLayer(a_collidableB);
      auto getcolliderA = GetTESObjectREFR(a_collidableA);
      auto getcolliderB = GetTESObjectREFR(a_collidableB);
      if (getcolliderA != nullptr && getcolliderB != nullptr) {
        if (getcolliderA->formID == 0x14 || getcolliderB->formID == 0x14) {
          log::info("Collision Layer of {} and {} : {} {}", getcolliderA->GetDisplayFullName(), getcolliderB->GetDisplayFullName()
            , static_cast<int>(colLayerA), static_cast<int>(colLayerB));
        }
      }

      if (getcolliderA->formID == 0x14 || getcolliderB->formID == 0x14) {
        if (getcolliderA == Runtime::GetExplosion("footstepExplosion") || getcolliderB == Runtime::GetExplosion("footstepExplosion")) {
          log::info("Object is explosion");
          *a_result = false;
        }
      }

      if (colLayerA == COL_LAYER::kBiped || colLayerA == COL_LAYER::kCharController || colLayerA == COL_LAYER::kDeadBip || colLayerA == COL_LAYER::kBipedNoCC) {
        auto colLayerB = GetCollisionLayer(a_collidableB);
        if (colLayerB == COL_LAYER::kBiped || colLayerB == COL_LAYER::kCharController || colLayerB == COL_LAYER::kDeadBip || colLayerB == COL_LAYER::kBipedNoCC || colLayerB == COL_LAYER::kSpellExplosion) {
          auto objA = GetTESObjectREFR(a_collidableA);
          if (objA) {
            auto objB = GetTESObjectREFR(a_collidableB);
            if (objB) {
              if (objA != objB)  {
                log::info("Collsion between: {} and {}", objA->GetDisplayFullName(), objB->GetDisplayFullName());
                if (IsCollisionDisabledBetween(objA, objB)) {
                  log::info("Collision is disabled");
                  *a_result = false;
                }
              }
            }
          }
        }
      }
    }
    return a_result;
  }
}
