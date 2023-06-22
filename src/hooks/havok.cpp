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

    REL::Relocation<std::uintptr_t> Vtbl{ RE::VTABLE_hkpCollidableCollidableFilter[0] };
		_IsCollisionEnabled = Vtbl.write_vfunc(0x0, IsCollisionEnabled);
	}

	void Hook_Havok::ProcessHavokHitJobs(void* a1)
	{
		_ProcessHavokHitJobs(a1);

		EventDispatcher::DoHavokUpdate();
	}

  // Credit: FlyingParticle for code on getting the NiAvObject
  //         maxsu. for IsCollisionEnabled idea
  bool Hook_Havok::IsCollisionEnabled(hkpCollidableCollidableFilter* a_this, const hkpCollidable& a_collidableA, const hkpCollidable& a_collidableB) {
    void* owner = a_collidableA.GetOwner();
    if (owner) {
      auto name = GetRawName(owner);
      log::info("IsCollisionEnabled: {}", name);
      hkpWorldObject* obj = a_collidableA.GetOwner<hkpWorldObject>();
      if (obj) {
        auto tesObj = obj->GetUserData();
        if (tesObj) {
          log::info("tesObj: {}", tesObj->GetDisplayFullName());
        }
      }
    }
    return _IsCollisionEnabled(a_this, a_collidableA, a_collidableB);
  }
}
