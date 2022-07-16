#include "hooks/timedelta.h"
#include "util.h"
#include "GtsManager.h"
#include "persistent.h"

using namespace RE;
using namespace SKSE;
using namespace Gts;

namespace Hooks
{
	void Hook_TimeDelta::Hook() {
		logger::info("Hooking Time Delta");
		REL::Relocation<std::uintptr_t> Vtbl{ RE::VTABLE_BSTAnimationGraphDataChannel_Actor_float_ActorTimeDeltaChannel_[0] };

		_PollChannelUpdateImpl = Vtbl.write_vfunc(0x01, PollChannelUpdateImpl);
	}

	void Hook_TimeDelta::PollChannelUpdateImplu(RE::BSAnimationGraphChannel* a_this, bool poll) {
		log::info("Hook Time Delta: {}", poll);
        log::info("Hook Time Delta (int): {}", reinterpret_cast<int &>(poll));
        log::info("Hook Time Delta (float): {}", reinterpret_cast<float &>(poll));
		
		_PollChannelUpdateImpl(a_this, poll);
	}
}
