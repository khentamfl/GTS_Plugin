#include "managers/GrowthTremorManager.hpp"
#include "managers/GtsManager.hpp"
#include "magic/effects/SizePotion.hpp"
#include "magic/effects/common.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"
#include "util.hpp"

namespace Gts {
	std::string SizePotion::GetName() {
		return "SizePotion";
	}


    SizePotion::SizePotion(ActiveEffect* effect) : Magic(effect) {

		auto base_spell = GetBaseEffect();
		auto& runtime = Runtime::GetSingleton();

		if (base_spell == runtime.EffectSizePotionStrong) {
            this->Strenght = 0.20;
        }
        else if (base_spell == runtime.EffectSizePotionNormal) {
            this->Strenght = 0.15;
        }
        if (base_spell == runtime.EffectSizePotionWeak) {
            this->Strenght = 0.10;
        }
    }


	bool SizePotion::StartEffect(EffectSetting* effect) { 
		auto& runtime = Runtime::GetSingleton();
		return (effect == runtime.EffectSizePotionWeak || effect ==  runtime.EffectSizePotionNormal || effect == runtime.EffectSizePotionStrong);
	}

	void SizePotion::OnStart() {
		auto caster = GetCaster();
        if (!caster) {
			return;
		}
        auto& runtime = Runtime::GetSingleton();
        auto saved_data = Persistent::GetSingleton().GetActorData(caster);
        float PotionPower = this->Strength;
        float BonusSize = runtime.sizeLimit->value * PotionPower;
        saved_data->max_scale += BonusSize;
	}

	void SizePotion::OnUpdate() {
	}

	void SizePotion::OnFinish() {
        auto caster = GetCaster();
        if (!caster) {
			return;
		}
        auto& runtime = Runtime::GetSingleton();
        auto saved_data = Persistent::GetSingleton().GetActorData(caster);
        float PotionPower = this->Strength;
        float BonusSize = runtime.sizeLimit->value * PotionPower;
        saved_data->max_scale -= BonusSize;
		auto caster = GetCaster();
	}
}