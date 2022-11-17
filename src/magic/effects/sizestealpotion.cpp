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
		return effect == Runtime::GetMagicEffect("EffectSizeHungerPotion");
	}

	void SizeHunger::OnStart() {
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
		auto caster = GetCaster();

		if (!caster) {
			return;
		}
		float Power = SizeManager::GetSingleton().GetSizeHungerBonus(caster);
		SizeManager::GetSingleton().ModSizeHungerBonus(caster, -Power);
	}
}
