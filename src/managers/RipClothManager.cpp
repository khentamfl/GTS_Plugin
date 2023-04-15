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
	ClothManager& ClothManager::GetSingleton() noexcept {
		static ClothManager instance;
		return instance;
	}

	std::string ClothManager::DebugName() {
		return "ClothManager";
	}

	void ClothManager::CheckRip() {
		if (Runtime::GetFloat("AllowClothTearing") == 0.0) {
			return; // Abort doing anything if not set to 1
		}
		static Timer timer = Timer(0.1);
		if (timer.ShouldRunFrame()) {

			auto player = PlayerCharacter::GetSingleton();
			float scale = get_visual_scale(player);

			std::vector<BGSBipedObjectForm::BipedObjectSlot> slots = {
				BGSBipedObjectForm::BipedObjectSlot::kHead,
				//BGSBipedObjectForm::BipedObjectSlot::kHair,
				BGSBipedObjectForm::BipedObjectSlot::kBody,
				BGSBipedObjectForm::BipedObjectSlot::kHands,
				BGSBipedObjectForm::BipedObjectSlot::kForearms,
				BGSBipedObjectForm::BipedObjectSlot::kAmulet,
				BGSBipedObjectForm::BipedObjectSlot::kRing,
				BGSBipedObjectForm::BipedObjectSlot::kFeet,
				BGSBipedObjectForm::BipedObjectSlot::kCalves,
				// BGSBipedObjectForm::BipedObjectSlot::kShield,
				// BGSBipedObjectForm::BipedObjectSlot::kTail,
				// BGSBipedObjectForm::BipedObjectSlot::kLongHair,
				BGSBipedObjectForm::BipedObjectSlot::kCirclet,
				// BGSBipedObjectForm::BipedObjectSlot::kEars,
				// BGSBipedObjectForm::BipedObjectSlot::kModMouth,
				BGSBipedObjectForm::BipedObjectSlot::kModNeck,
				BGSBipedObjectForm::BipedObjectSlot::kModChestPrimary,
				BGSBipedObjectForm::BipedObjectSlot::kModBack,
				BGSBipedObjectForm::BipedObjectSlot::kModMisc1,
				BGSBipedObjectForm::BipedObjectSlot::kModPelvisPrimary,
				// BGSBipedObjectForm::BipedObjectSlot::kDecapitateHead,
				// BGSBipedObjectForm::BipedObjectSlot::kDecapitate,
				BGSBipedObjectForm::BipedObjectSlot::kModPelvisSecondary,
				BGSBipedObjectForm::BipedObjectSlot::kModLegRight,
				BGSBipedObjectForm::BipedObjectSlot::kModLegLeft,
				// BGSBipedObjectForm::BipedObjectSlot::kModFaceJewelry,
				BGSBipedObjectForm::BipedObjectSlot::kModChestSecondary,
				BGSBipedObjectForm::BipedObjectSlot::kModShoulder,
				BGSBipedObjectForm::BipedObjectSlot::kModArmLeft,
				BGSBipedObjectForm::BipedObjectSlot::kModArmRight,
				// BGSBipedObjectForm::BipedObjectSlot::kModMisc2,
				// BGSBipedObjectForm::BipedObjectSlot::kFX01,
			};
			std::vector<TESObjectARMO*> armors = {};
			for (auto slot: slots) {
				auto armor = player->GetWornArmor(slot);
				if (armor) {
					armors.push_back(armor);
				}
			}
			auto count = armors.size();
			if (count == 0) {
				return; // Naked
			}
			auto itemIdx = rand() % count;
			auto ArmorSlot = armors[itemIdx];


			//log::info("Armor Slot: {}", ArmorSlot);
			if (!player || scale <= 2.5) {
				return;
			}

			if (scale < this->clothtearthreshold - 0.5 && this->clothtearthreshold > 2.5) {
				this->clothtearthreshold = 2.5; // reset stuff
			}
			if (this->clothtearthreshold < 2.5) {
				this->clothtearthreshold = 2.5;
			}
			if (ArmorSlot != nullptr && scale >= this->clothtearthreshold) {
				log::info("Unequipping armor");
				this->clothtearthreshold += 0.10 * ((rand() % 7) + 1.0);

				auto manager = RE::ActorEquipManager::GetSingleton();
				manager->UnequipObject(player, ArmorSlot);

				Runtime::PlaySound("ClothTearSound", player, 1.0, 1.0);
				Runtime::PlaySoundAtNode("MoanSound", player, 1.0, 1.0, "NPC Head [Head]");
				Rumble::Once("ClothManager", player, 32 * scale, 0.05);
				log::info("Cloth Tearing Success. Threshold: {}, count: {}, Unequipped Armor: {}", this->clothtearthreshold, this->clothtearcount, ArmorSlot->GetFullName());
			}
		}
	}
}
