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
    Control_Map* Control_Map::GetSingleton()
	{
		REL::Relocation<Control_Map**> singleton{ Offset::Control_Map::Singleton };
		return *singleton;
	}
}

namespace Gts {


	void ToggleControls(UEFlag a_flag, bool a_enable) {
        // Pretty much the same as CommonLib one, but without sending Events
        // Since they break sneaking for some reason, so we don't want to use them
		auto controlMap = Control_Map::GetSingleton();
		if (controlMap) { 
			if (a_enable) {
                controlMap->GetRuntimeData().enabledControls.set(a_flags);
                if (controlMap->GetRuntimeData().unk11C != UEFlag::kInvalid) {
                    controlMap->GetRuntimeData().unk11C.set(a_flags);
                }
            } else {
                controlMap->GetRuntimeData().enabledControls.reset(a_flags);
                if (controlMap->GetRuntimeData().unk11C != UEFlag::kInvalid) {
                    controlMap->GetRuntimeData().unk11C.reset(a_flags);
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
                    auto controlMap = ControlMap::GetSingleton();
		            if (controlMap) { 
                        ToggleControls(UEFlag::kFighting, !GtsBusy);
                        ToggleControls(UEFlag::kActivate, !GtsBusy);
                        ToggleControls(UEFlag::kMovement, !GtsBusy);
                        //ToggleControls(UEFlag::kSneaking, !GtsBusy);
                        ToggleControls(UEFlag::kJumping, !GtsBusy);
                        log::info("Adjusting Controls");
                    }
                }
            }
        }
    }
}	
