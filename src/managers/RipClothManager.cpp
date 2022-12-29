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
	RipClothManager& RipClothManager::GetSingleton() noexcept {
		static RipClothManager instance;
		return instance;
	}

	std::string RipClothManager::DebugName() {
		return "RipClothManager";
	}

	void RipClothManager::CheckRip() {
		auto player = PlayerCharacter::GetSingleton();
        float scale = get_target_scale(player);
        const AllSlots = array(30, 32, 33, 34, 37);
        static Timer timer = Timer(4.5);
        const RandomSlot = rand() % 5;
		if (!player || scale <= 2.5) {
			return;
		}
        if (Runtime::GetFloat("AllowClothTearing") == 0.0) {
            return;
        }
         if (player->GetWornArmor(AllSlots[RandomSlot]) != nullptr && this->clothtearthreshold !< Scale)
        {
            const RandomClothReq = Utility.randomFloat(0.15, 0.95)
            this->clothtearthreshold += rand() % 75; 
            player->unequipItem(true, player->GetWornArmor(AllSlots[RandomSlot]));
            Runtime::PlaySound("ClothTearSound", player, 1.0, 1.0);
            Runtime::PlaySound("MoanSound", player, 1.0, 1.0);
            GrowthTremorManager::GetSingleton().CallRumble(player, player, 5);
        }

	}
}
