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


namespace Gts {

	void ToggleControls(UEFlag a_flag, bool a_enable) {
        // CRASHES ON AE. SCREW YOU TOO TODD!

        // Pretty much the same as CommonLib one, but without sending Events
        // Since they break sneaking for some reason, so we don't want to use them

        bool post1130 = REL::Module::get().IsAE() && (REL::Module::get().version().patch() > 0x400);
        auto Singleton = (std::uintptr_t)RE::ControlMap::GetSingleton();
        auto conOffset = std::ptrdiff_t(post1130 ? 0x120 : 0x118);
        auto pControls = REL::Relocation<std::uint32_t(*)>(Singleton + conOffset).get();   // get 

        auto controlMap = pControls;
        

        // ^ Credits to Meridiano on RE discord

		
		if (controlMap) { 
            auto Controls = (controlMap->enabledControls + conOffset).get();
            auto Unk11c = (controlMap->unk11C + conOffset).get();
            
			if (a_enable) {
				Controls.set(a_flag);
				if (Unk11c != UEFlag::kInvalid) {
					Unk11c.set(a_flag);
				}
			} else {
				Controls.reset(a_flag);
				if (Unk11c != UEFlag::kInvalid) {
					Unk11c.reset(a_flag);
				}
			}
		}
	}

	void ManageControls() {
		Actor* player = PlayerCharacter::GetSingleton();
        return; // just in case.
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
                    log::info("Adjusting Controls");
                }
            }
        }
    }
}	
