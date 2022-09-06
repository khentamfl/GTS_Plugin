#include "managers/GrowthTremorManager.hpp"
#include "managers/GtsSizeManager.hpp"
#include "magic/effects/common.hpp"
#include "managers/GtsManager.hpp"
#include "data/persistent.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "data/time.hpp"
#include "timer.hpp"
#include "util.hpp"
#include "node.hpp"


using namespace RE;
using namespace Gts;


namespace Gts {
	SizeManager& SizeManager::GetSingleton() noexcept {
		static SizeManager instance;
		return instance;
	}

	inline float SizeManager::GetRaceScale(Actor* actor) {
		auto GetNode = find_node(actor, "NPC", false);
		float NodeScale = GetNode->world.scale;
		return GetNode ? NodeScale : 1; // <- not used, causes troubles with quest progression. (Can't reach 1.44 for example when 1.50 is needed.)
	}

	void SizeManager::UpdateSize(Actor* actor) {
		auto& runtime = Runtime::GetSingleton();
		float Gigantism = this->GetEnchantmentBonus(actor)/100;
		float GetLimit = clamp(1.0, 99999999.0, runtime.sizeLimit->value);
		float Persistent_Size = Persistent::GetSingleton().GetData(actor)->bonus_max_size;
		float SelectedFormula = runtime.SelectedSizeFormula->value;
		if (SelectedFormula >= 2.0) {
			GetLimit = clamp(1.0, 99999999.0, runtime.MassBasedSizeLimit->value);
		}
		float RaceScale = GetRaceScale(actor);
		float RaceScaleLimit = (RaceScale * (GetLimit + Persistent_Size)) * (1.0 + Gigantism);
		if (actor->formID==0x14) {
			log::info("PC Race Scale: {}, SizeLimit: {}, Persistent: {}, Scale Limit with Race: {}", RaceScale, GetLimit, Persistent_Size, RaceScaleLimit);
		}
		//float TotalLimit = (RaceScale * (GetLimit + Persistent_Size)) * (1.0 + Gigantism);
		float TotalLimit = (GetLimit + Persistent_Size) * (1.0 + Gigantism);



		if (TotalLimit < 1.0) {
			TotalLimit = 1.0;
		}
		if (get_max_scale(actor) < TotalLimit || get_max_scale(actor) > TotalLimit) {
			set_max_scale(actor, TotalLimit);
			log::info("Total limit of {} is: {}", actor->GetDisplayFullName(), TotalLimit);
		}
	}
	void SizeManager::SetEnchantmentBonus(Actor* actor, float amt) {
		this->GetData(actor).enchantmentBonus = amt;
	}

	float SizeManager::GetEnchantmentBonus(Actor* actor) {
		if (this->GetData(actor).enchantmentBonus <= 0.0) {
			return 0.0; //Protect against 0
		}
		return this->GetData(actor).enchantmentBonus;
	}

	void SizeManager::ModEnchantmentBonus(Actor* actor, float amt) {
		this->GetData(actor).enchantmentBonus += amt;
	}

	SizeManagerData& SizeManager::GetData(Actor* actor) {
		this->sizeData.try_emplace(actor);
		return this->sizeData.at(actor);
	}
}
