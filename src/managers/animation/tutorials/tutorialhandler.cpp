#include "managers/animation/AnimationManager.hpp"
#include "managers/animation/tutorials/tutorialhandler.hpp"
#include "managers/damage/AccurateDamage.hpp"
#include "managers/GtsSizeManager.hpp"
#include "magic/effects/common.hpp"
#include "utils/papyrusUtils.hpp"
#include "managers/explosion.hpp"
#include "managers/footstep.hpp"
#include "utils/actorUtils.hpp"
#include "utils/findActor.hpp"
#include "data/persistent.hpp"
#include "data/transient.hpp"
#include "data/runtime.hpp"
#include "spring.hpp"
#include "scale/scale.hpp"
#include "colliders/RE.hpp"
#include "colliders/actor.hpp"
#include "timer.hpp"
#include "node.hpp"
#include "utils/av.hpp"
#include "colliders/RE.hpp"

using namespace RE;
using namespace Gts;

namespace Gts {
	/*void TutorialMessage(std::string_view message, std::string_view type) {
        bool AllowMessages = Persistent::GetSingleton().AllowTutorials;
        if (!AllowMessages) {
            return;
        }
        bool HugsSeen = Persistent::GetSingleton().HugsTutorialSeen;
        bool CrushSeen = Persistent::GetSingleton().CrushTutorialSeen;
        bool GrabSeen = Persistent::GetSingleton().GrabTutorialSeen;
        bool CalamitySeen = Persistent::GetSingleton().CalamityTutorialSeen;
        if (type == "Hugs" && !HugsSeen) { 
            MessageBox(message);
            Hugs = true;
        } else if (type == "Crush" && !CrushSeen) {

        } else if (type == "Grab" && !GrabSeen) {

        } else if (type == "Calamity" && !CalamitySeen) {

        }
		
	}*/
}