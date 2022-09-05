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


using namespace RE;
using namespace Gts;


namespace Gts {
	SizeManager& SizeManager::GetSingleton() noexcept {
		static SizeManager instance;
		return instance;
	}
	void SizeManager::UpdateSize(Actor* actor) {
		auto& runtime = Runtime::GetSingleton();
		float Gigantism = this->GetEnchantmentBonus(actor)/100;
		float GetLimit = clamp(1.0, 99999999.0, runtime.sizeLimit->value);
		float Persistent_Size = Persistent::GetSingleton().GetData(actor)->bonus_max_size;
		float TotalLimit = (GetLimit + Persistent_Size) * (1.0 + Gigantism);
		if (get_max_scale(actor) < TotalLimit || get_max_scale(actor) > TotalLimit) {
			set_max_scale(actor, TotalLimit);
			log::info("Total limit of {} is: {}", actor->GetDisplayFullName(), TotalLimit);
			//log::info("{} _ size limit is set to {}", actor->GetDisplayFullName(), TotalLimit);
		}
	}
	void SizeManager::SetEnchantmentBonus(Actor* actor, float amt) {
		this->GetData(actor).enchantmentBonus = amt;
	}

	float SizeManager::GetEnchantmentBonus(Actor* actor) {
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
