#include "managers/RipClothManager.hpp"
#include "managers/GrowthTremorManager.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/GtsManager.hpp"
#include "magic/effects/common.hpp"
#include "util.hpp"
#include "scale/scale.hpp"
#include "data/persistent.hpp"
#include "data/runtime.hpp"
#include "data/time.hpp"
#include "timer.hpp"
#include "timer.hpp"

using namespace RE;
using namespace Gts;


namespace Gts {
	ClothManager& ClothManager::GetSingleton() noexcept {
		static ClothManager instance;
		return instance;
	}

    std::string ClothManager::DebugName() {
		return "ClothManager";
	}

	void ClothManager::CheckRip() {
		auto player = PlayerCharacter::GetSingleton();
        float scale = get_target_scale(player);
        int AllSlots[] = {30, 32, 33, 34, 37}; // Should strip only these slots
        static Timer timer = Timer(4.5);
        const RandomSlot = rand() % 5; // Randomly choose slot to strip
        if (timer.ShouldRunFrame()) {
		    if (!player || scale <= 2.5) {
			    return;
		    }
             if (Runtime::GetFloat("AllowClothTearing") == 0.0) {
                 return; // Return of not set to 1
             }
             if (this->clothtearcount >= 5.0) {
                this->clothtearcount = 0.0;
                this->clothtearthreshold = 0.0; // reset stuff
             }
             if (player->GetWornArmor(AllSlots[RandomSlot]) != nullptr && this->clothtearthreshold !< Scale)
            {
                const RandomClothReq = Utility.randomFloat(0.15, 0.95)
                this->clothtearthreshold += rand() % 75; 
                this->clothtearcount +=1.0;
                player->unequipItem(true, player->GetWornArmor(AllSlots[RandomSlot]));
                Runtime::PlaySound("ClothTearSound", player, 1.0, 1.0);
                Runtime::PlaySound("MoanSound", player, 1.0, 1.0);
                GrowthTremorManager::GetSingleton().CallRumble(player, player, 5);

            }
	    }
    }
}
