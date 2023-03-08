#include "managers/GtsManager.hpp"
#include "managers/GrowthTremorManager.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"
#include "data/persistent.hpp"
#include "data/transient.hpp"
#include "magic/effects/common.hpp"

namespace Gts {
	GrowthTremorManager& GrowthTremorManager::GetSingleton() noexcept {
		static GrowthTremorManager instance;
		return instance;
	}
}
