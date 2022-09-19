		
        
#include "managers/RandomGrowth.hpp"
#include "managers/GrowthTremorManager.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/InputManager.hpp"
#include "magic/effects/common.hpp"
#include "util.hpp"
#include "scale/scale.hpp"
#include "data/persistent.hpp"
#include "data/runtime.hpp"
#include "data/time.hpp"
#include "timer.hpp"

using namespace RE;
using namespace Gts;


namespace Gts {
	InputManager& InputManager::GetSingleton() noexcept {
		static InputManager instance;
		return instance;
	}
}

namespace {
        void DetectInput() {
        if (GetActionString() != "activate") {
			return;
		}
		else if (GetActionString() == "activate")
		{
			ConsoleLog::GetSingleton()->Print("E Pressed");
			mod_target_scale(PlayerCharacter::GetSingleton(),0.33);
			}
		else if (GetActionString() == "leftAttack" || GetActionString() == "rightAttack") {
			PlayerCharacter::GetSingleton()->NotifyAnimationGraph("JumpLand");
		    }
        }


        std::string GetActionString() {
			const auto ActivateButton = static_cast<RE::ButtonEvent*>(static_cast<RE::InputEvent*>((0x45)));
			const auto AttackLeftButton = static_cast<RE::ButtonEvent*>(static_cast<RE::InputEvent*>((0x01)));
			const auto AttackRightButton = static_cast<RE::ButtonEvent*>(static_cast<RE::InputEvent*>((0x02)));
			if (ActivateButton->IsPressed()) {
				return "activate";
			}
			else if (AttackLeftButton->IsPressed()) {
				return "leftAttack";
			}
			else if (AttackRightButton->IsPressed()) {
				return "rightAttack";
			} 
			else
            return "None";
    }
}

