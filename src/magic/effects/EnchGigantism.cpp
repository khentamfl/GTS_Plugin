#include "magic/effects/EnchGigantism.hpp"
#include "magic/effects/common.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/persistent.hpp"
#include "data/runtime.hpp"
#include "managers/GtsSizeManager.hpp"

namespace Gts {
	std::string Gigantism::GetName() {
		return "Gigantism";
	}

	void Gigantism::OnStart() {
		auto caster = GetCaster();

		if (!caster) {
			return;
		}
		this->magnitude = GetActiveEffect()->magnitude;
		float GigantismPower = this->magnitude;
		SizeManager::GetSingleton().ModEnchantmentBonus(caster, GigantismPower);
		//log::info("Increasing GigantismPower: {}, Actor: {}", SizeManager::GetSingleton().GetEnchantmentBonus(caster), caster->GetDisplayFullName());
		//log::info("Starting effect: {}", reinterpret_cast<std::uintptr_t>(GetActiveEffect()));
		//log::info("Total GigantismPower: {}, {}", SizeManager::GetSingleton().GetEnchantmentBonus(caster), caster->GetDisplayFullName());
	}

	void Gigantism::OnUpdate() {
	}


	void Gigantism::OnFinish() {
		auto caster = GetCaster();

		if (!caster) {
			return;
		}
		float GigantismPower = this->magnitude;
		SizeManager::GetSingleton().ModEnchantmentBonus(caster, -GigantismPower);
		//log::info("Decreasing GigantismPower: {}, Actor: {}", SizeManager::GetSingleton().GetEnchantmentBonus(caster), caster->GetDisplayFullName());
		//log::info("Stopping effect: {}", reinterpret_cast<std::uintptr_t>(GetActiveEffect()));
		//log::info("elapsedSeconds: {}, duration: {}", GetActiveEffect()->elapsedSeconds, GetActiveEffect()->duration);
		//log::info("Total GigantismPower: {}, {}", SizeManager::GetSingleton().GetEnchantmentBonus(caster), caster->GetDisplayFullName());
	}
}
