#include "managers/GtsSizeManager.hpp"
#include "managers/GrowthTremorManager.hpp"
#include "managers/GtsManager.hpp"
#include "magic/effects/common.hpp"
#include "util.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"
#include "data/time.hpp"
#include "timer.hpp"

using namespace RE;
using namespace Gts;


namespace Gts {
	SizeManager& SizeManager::GetSingleton() noexcept {
		static RandomGrowth instance;
		return instance;
	}
	void SizeManager::UpdateSize(Actor* Target) {
		auto& runtime = Runtime::GetSingleton();
        float Persistent_Size = Persistent::GetSingleton().GetActorData(Target)->bonus_max_size;
        float GetLimit = clamp(1.0, 99999999, runtime.sizeLimit);
        float TotalLimit = GetLimit + Persistent_Size;

        set_max_scale(Target, TotalLimit);

        log::info("Actor's {} _ size limit is set to {}", Target->GetDisplayFullName(), TotalLimit);
    }
}
