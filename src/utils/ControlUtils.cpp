#include "utils/ControlUtils.hpp"
#include "utils/actorUtils.hpp"
#include "data/transient.hpp"

using namespace Gts;
using namespace RE;
using namespace SKSE;
using namespace std;

namespace Gts {
	void ToggleControls(UEFlag a_flag, bool a_enable) {
        // Pretty much the same as CommonLib one, but without sending Events
        // Since they break sneaking for some reason, so we don't want to use them
		auto controlMap = ControlMap::GetSingleton();
		if (controlMap) { 
			if (a_enable) {
				//controlMap->enabledControls.set(a_flag);
				if (controlMap->unk11C != UEFlag::kInvalid) {
					controlMap->unk11C.set(a_flag);
				}
			} else {
				//controlMap->enabledControls.reset(a_flag);
				if (controlMap->unk11C != UEFlag::kInvalid) {
					controlMap->unk11C.reset(a_flag);
				}
			}
		}
	}

	void ManageControls() {
		Actor* player = PlayerCharacter::GetSingleton();
		if (player) {
            bool GtsBusy = IsGtsBusy(player) || IsTransitioning(player);
            bool AnimsInstalled = AnimationsInstalled(player);
            if (!AnimsInstalled) {
                return;
            }
            auto transient = Transient::GetSingleton().GetData(player);
            if (transient) {
                bool NeedsChange = transient->DisableControls;
                if (NeedsChange != GtsBusy) {
                    transient->DisableControls = GtsBusy; // switch it
                    ToggleControls(UEFlag::kFighting, !GtsBusy);
                    ToggleControls(UEFlag::kActivate, !GtsBusy);
                    ToggleControls(UEFlag::kMovement, !GtsBusy);
                    ToggleControls(UEFlag::kSneaking, !GtsBusy);
                    ToggleControls(UEFlag::kJumping, !GtsBusy);
                    log::info("Adjusting Controls");
                }
            }
        }
    }
}	
