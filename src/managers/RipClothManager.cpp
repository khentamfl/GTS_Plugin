#include "managers/RipClothManager.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/GtsManager.hpp"
#include "magic/effects/common.hpp"
#include "scale/scale.hpp"
#include "data/persistent.hpp"
#include "data/runtime.hpp"
#include "data/time.hpp"
#include "timer.hpp"
#include "timer.hpp"
#include "managers/Rumble.hpp"

using namespace RE;
using namespace Gts;

namespace Gts {

    //List Of Keywords (Editor ID's) we want to ignore when stripping
	static std::vector<string> KeywordBlackList = {"SOS_Genitals"};

	//Why Constantly Create A List on the stack, Just Define once and reference.
    static std::vector<BGSBipedObjectForm::BipedObjectSlot> VallidSlots = {
        BGSBipedObjectForm::BipedObjectSlot::kHead,					// 30
        // BGSBipedObjectForm::BipedObjectSlot::kHair,				// 31
        BGSBipedObjectForm::BipedObjectSlot::kBody,					// 32
        BGSBipedObjectForm::BipedObjectSlot::kHands,				// 33
        BGSBipedObjectForm::BipedObjectSlot::kForearms,				// 34
        // BGSBipedObjectForm::BipedObjectSlot::kAmulet,			// 35
        // BGSBipedObjectForm::BipedObjectSlot::kRing,				// 36
        BGSBipedObjectForm::BipedObjectSlot::kFeet,					// 37
        BGSBipedObjectForm::BipedObjectSlot::kCalves,				// 38
        // BGSBipedObjectForm::BipedObjectSlot::kShield,			// 39
        // BGSBipedObjectForm::BipedObjectSlot::kTail,				// 40
        // BGSBipedObjectForm::BipedObjectSlot::kLongHair,			// 41
        BGSBipedObjectForm::BipedObjectSlot::kCirclet,				// 42
        BGSBipedObjectForm::BipedObjectSlot::kEars,					// 43
        BGSBipedObjectForm::BipedObjectSlot::kModMouth,				// 44
        BGSBipedObjectForm::BipedObjectSlot::kModNeck,				// 45
        BGSBipedObjectForm::BipedObjectSlot::kModChestPrimary,		// 46
        BGSBipedObjectForm::BipedObjectSlot::kModBack,				// 47
        BGSBipedObjectForm::BipedObjectSlot::kModMisc1,				// 48
        BGSBipedObjectForm::BipedObjectSlot::kModPelvisPrimary,		// 49
        // BGSBipedObjectForm::BipedObjectSlot::kDecapitateHead,	// 50
        // BGSBipedObjectForm::BipedObjectSlot::kDecapitate,		// 51
        BGSBipedObjectForm::BipedObjectSlot::kModPelvisSecondary,   // 52
        // BGSBipedObjectForm::BipedObjectSlot::kModLegRight,		// 53
        BGSBipedObjectForm::BipedObjectSlot::kModLegLeft,			// 54
        // BGSBipedObjectForm::BipedObjectSlot::kModFaceJewelry,	// 55
        BGSBipedObjectForm::BipedObjectSlot::kModChestSecondary,	// 56
        BGSBipedObjectForm::BipedObjectSlot::kModShoulder,			// 57
        // BGSBipedObjectForm::BipedObjectSlot::kModArmLeft,		// 58
        BGSBipedObjectForm::BipedObjectSlot::kModArmRight,			// 59
        // BGSBipedObjectForm::BipedObjectSlot::kModMisc2,			// 60
        // BGSBipedObjectForm::BipedObjectSlot::kFX01,				// 61
    };


	ClothManager& ClothManager::GetSingleton() noexcept {
		static ClothManager instance;
		return instance;
	}

	std::string ClothManager::DebugName() {
		return "ClothManager";
	}

	std::vector<TESObjectARMO*> ParseArmors(RE::Actor *Act) {
        std::vector<TESObjectARMO *> ArmorList;
		for (auto Slot : VallidSlots) {
            
            auto Armor = Act->GetWornArmor(Slot);
            // If armor is null skip
            if (!Armor) continue;

			for (const auto &BKwd : KeywordBlackList) {
                if (Armor->HasKeywordString(BKwd)) {
					continue;	//If blacklisted keyword is found skip
				}
                ArmorList.push_back(Armor);						//Else Add it to the vector
            }
        }
        return ArmorList;
	}

	void ClothManager::CheckRip() {
		if (Runtime::GetFloat("AllowClothTearing") == 0.0)  {
			return; // Abort doing anything if not set to 1
		}
		
		static Timer timer = Timer(0.33);
		if (timer.ShouldRunFrame()) {

			auto player = PlayerCharacter::GetSingleton();
			float scale = get_visual_scale(player);

            std::vector<TESObjectARMO *> armors = ParseArmors(player);

			auto count = armors.size();
			if (count == 0) return; // Naked
			
			
			auto itemIdx = rand() % count;
			auto ArmorSlot = armors[itemIdx];

			if (!player || scale <= 1.5) return; // Not meeting requirements

			if (scale < this->clothtearthreshold - 0.5 && this->clothtearthreshold > 2.5) 
				this->clothtearthreshold = 1.5; // reset stuff
			
			if (this->clothtearthreshold < 1.5) 
				this->clothtearthreshold = 1.5;

			if (ArmorSlot != nullptr && scale >= this->clothtearthreshold) {
				this->clothtearthreshold += 0.10 * ((rand() % 7) + 1.0);
				auto manager = RE::ActorEquipManager::GetSingleton();
				manager->UnequipObject(player, ArmorSlot);
				Runtime::PlaySound("ClothTearSound", player, 1.0, 1.0);
				Runtime::PlaySoundAtNode("MoanSound", player, 1.0, 1.0, "NPC Head [Head]");
				Rumble::Once("ClothManager", player, 32 * scale, 0.05);
			}
		}
	}
}