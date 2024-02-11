#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/animation/Utils/CrawlUtils.hpp"
#include "managers/emotions/EmotionManager.hpp"
#include "managers/animation/Grab_Sneak.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/ai/aifunctions.hpp"
#include "managers/CrushManager.hpp"
#include "utils/papyrusUtils.hpp"
#include "utils/actorUtils.hpp"
#include "data/persistent.hpp"
#include "managers/explosion.hpp"
#include "managers/footstep.hpp"
#include "managers/Rumble.hpp"
#include "managers/tremor.hpp"
#include "data/transient.hpp"
#include "managers/vore.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "node.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace {
    void GTS_Sneak_Vore_Grab_Start(AnimationEventData& data) { // Register Tiny for Vore
		auto otherActor = Grab::GetHeldActor(&data.giant);

        ManageCamera(&data.giant, true, CameraTracking::Grab_Left);
		auto& VoreData = Vore::GetSingleton().GetVoreData(&data.giant);

		if (otherActor) {
			VoreData.AddTiny(otherActor);
		}
	}

    void GTS_Sneak_Vore_Grab_Eat(AnimationEventData& data) { 
		auto& VoreData = Vore::GetSingleton().GetVoreData(&data.giant);
		for (auto& tiny: VoreData.GetVories()) {
			tiny->NotifyAnimationGraph("JumpFall");
			Attacked(tiny, &data.giant);
			VoreData.GrabAll(); // Switch to AnimObjectA attachment
		}
		ManageCamera(&data.giant, true, CameraTracking::Hand_Right);
	}
    // Rest is handled inside Vore_Sneak (some events are re-used)
}

namespace Gts {
    void Animation_GrabSneak::RegisterEvents() { 
		AnimationManager::RegisterEvent("GTS_Sneak_Vore_Grab_Start", "SneakVore", GTS_Sneak_Vore_Grab_Start);
        AnimationManager::RegisterEvent("GTS_Sneak_Vore_Grab_Eat", "SneakVore", GTS_Sneak_Vore_Grab_Eat);
    }
}