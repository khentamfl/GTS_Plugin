#include "managers/AnimationManager.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/GrowthTremorManager.hpp"
#include "managers/ShrinkToNothingManager.hpp"
#include "managers/CrushManager.hpp"
#include "managers/impact.hpp"
#include "magic/effects/common.hpp"
#include "managers/GtsManager.hpp"
#include "utils/actorUtils.hpp"
#include "data/persistent.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "data/time.hpp"
#include "events.hpp"
#include "timer.hpp"
#include "node.hpp"


using namespace RE;
using namespace Gts;



namespace Gts {
	AnimationManager& AnimationManager::GetSingleton() noexcept {
		static AnimationManager instance;
		return instance;
	}

	std::string AnimationManager::DebugName() {
		return "AnimationManager";
	}

	void AnimationManager::ActorAnimEvent(Actor* actor, const std::string_view& tag, const std::string_view& payload) {
        if (actor->formID == 0x14) {
            log::info("Tag: {}, payload: {}", tag, payload);
            if (tag == "GTSstompimpact") {
                log::info("Stomp is true");
            }
        }
    }
}