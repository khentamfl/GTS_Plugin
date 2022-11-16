#include "magic/effects/sizestealpotion.hpp"
#include "magic/effects/common.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/persistent.hpp"
#include "data/runtime.hpp"
#include "util.hpp"
#include "managers/GtsSizeManager.hpp"

namespace Gts {
	std::string SizeHunger::GetName() {
		return "SizeHunger";
	}

	bool SizeHunger::StartEffect(EffectSetting* effect) { // NOLINT
		auto& runtime = Runtime::GetSingleton();
		return effect == runtime.GetMagicEffect("EffectSizeHungerPotion");
	}

	void SizeHunger::OnStart() {
		auto& runtime = Runtime::GetSingleton();
		auto caster = GetCaster();

		if (!caster) {
			return;
		}
		float Power = GetActiveEffect()->magnitude;
		SizeManager::GetSingleton().SetSizeHungerBonus(caster, Power);
	}

	void SizeHunger::OnUpdate() {
	}


	void SizeHunger::OnFinish() {
		auto& runtime = Runtime::GetSingleton();
		auto caster = GetCaster();

		if (!caster) {
			return;
		}
		float Power = SizeManager::GetSingleton().GetSizeHungerBonus(caster);
		SizeManager::GetSingleton().ModSizeHungerBonus(caster, -Power);
	}
}