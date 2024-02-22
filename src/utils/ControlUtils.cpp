#include "utils/ControlUtils.hpp"
#include "utils/actorUtils.hpp"
#include "data/transient.hpp"

using namespace Gts;
using namespace RE;
using namespace SKSE;
using namespace std;

namespace {
    bool IsGtsBusy_ForControls(Actor* actor) {
		bool GTSBusy;
		actor->GetGraphVariableBool("GTS_Busy", GTSBusy);
        // Have to use this because Hand Swipes make original bool return false

		return GTSBusy;
	}
}

namespace RE {
    GTSControlMap* GTSControlMap::GetSingleton()
	{
		REL::Relocation<GTSControlMap**> singleton{RELOCATION_ID(514705, 400863)};
		return *singleton;
	}
}


namespace Gts {
	void ToggleControls(UEFlag a_flag, bool a_enable) {
        // CRASHES ON AE. SCREW YOU TOO TODD!

        // Pretty much the same as CommonLib one, but without sending Events
        // Since they break sneaking for some reason, so we don't want to use them
		auto controlMap = GTSControlMap::GetSingleton();
		if (controlMap) { 
			if (a_enable) {
				controlMap->GetRuntimeData().enabledControls.set(a_flag);
				if (controlMap->GetRuntimeData().unk11C != UEFlag::kInvalid) {
					controlMap->GetRuntimeData().unk11C.set(a_flag);
				}
			} else {
				controlMap->GetRuntimeData().enabledControls.reset(a_flag);
				if (controlMap->GetRuntimeData().unk11C != UEFlag::kInvalid) {
					controlMap->GetRuntimeData().unk11C.reset(a_flag);
				}
			}
		}
	}

	void ManageControls() {
		Actor* player = PlayerCharacter::GetSingleton();
		if (player) {
            bool GtsBusy = IsGtsBusy_ForControls(player) || IsTransitioning(player);
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
                    //ToggleControls(UEFlag::kSneaking, !GtsBusy);
                    ToggleControls(UEFlag::kJumping, !GtsBusy);
                   // log::info("Adjusting Controls");
                }
            }
        }
    }
}	
