#include "managers/animation/AnimationInputManager.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/GrowthTremorManager.hpp"
#include "managers/ShrinkToNothingManager.hpp"
#include "managers/CrushManager.hpp"
#include "managers/impact.hpp"
#include "magic/effects/common.hpp"
#include "managers/GtsManager.hpp"
#include "utils/actorUtils.hpp"
#include "data/persistent.hpp"
#include "data/transient.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "data/time.hpp"
#include "events.hpp"
#include "timer.hpp"
#include "node.hpp"

using namespace RE;
using namespace Gts;
using namespace std;

///This .cpp will be used to adjust player control during various animations, allowing to: 
// - crush on button press
// - Attack during idle loop through crossing legs
// - Grabbing Actors->Eating actors(?)/Crushing Actors with Fingers(?)/Putting actors down(?)/Throwing Actors(?)

namespace Gts {
	AnimationInputManager& AnimationInputManager::GetSingleton() noexcept {
		static AnimationInputManager instance;
		return instance;
	}

	std::string AnimationInputManager::DebugName() {
		return "AnimationInputManager";
	}
}

