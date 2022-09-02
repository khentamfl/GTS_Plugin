#include "managers/GrowthTremorManager.hpp"
#include "managers/GtsSizeManager.hpp"
#include "magic/effects/common.hpp"
#include "managers/GtsManager.hpp"
#include "data/persistent.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "data/time.hpp"
#include "timer.hpp"
#include "util.hpp"


using namespace RE;
using namespace Gts;


namespace Gts {
	SizeManager& SizeManager::GetSingleton() noexcept {
		static SizeManager instance;
		return instance;
	}
	void SizeManager::UpdateSize(Actor* Actor) {
		auto& runtime = Runtime::GetSingleton();
        float GetLimit = clamp(1.0, 99999999.0, runtime.sizeLimit->value);
        float Persistent_Size = Persistent::GetSingleton().GetActorData(Actor)->bonus_max_size;
        float TotalLimit = GetLimit + Persistent_Size;
        if (get_max_scale(Actor) < TotalLimit || get_max_scale(Actor) > TotalLimit) {
        set_max_scale(Actor, TotalLimit);
        log::info("{} _ size limit is set to {}", Actor->GetDisplayFullName(), TotalLimit);
        }
    }
}
