#include "managers/GrowthTremorManager.hpp"
#include "managers/GtsManager.hpp"
#include "magic/effects/SizePotion.hpp"
#include "magic/effects/common.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/persistent.hpp"
#include "data/runtime.hpp"
#include "util.hpp"

namespace Gts {
	std::string SizePotion::GetName() {
		return "SizePotion";
	}


    SizePotion::SizePotion(ActiveEffect* effect) : Magic(effect) {

		auto base_spell = GetBaseEffect();
		auto& runtime = Runtime::GetSingleton();

        if (base_spell == runtime.EffectSizePotionExtreme) {
            this->Strenght = 0.35;
        }
		else if (base_spell == runtime.EffectSizePotionStrong) {
            this->Strenght = 0.20;
        }
        else if (base_spell == runtime.EffectSizePotionNormal) {
            this->Strenght = 0.15;
        }
        else if (base_spell == runtime.EffectSizePotionWeak) {
            this->Strenght = 0.10;
        }
        log::info("Strenght is {}", this->Strenght);
    }


	bool SizePotion::StartEffect(EffectSetting* effect) { 
		auto& runtime = Runtime::GetSingleton();
		return (effect == runtime.EffectSizePotionStrong || effect ==  runtime.EffectSizePotionNormal || effect == runtime.EffectSizePotionWeak || effect == runtime.EffectSizePotionExtreme);
	}

	void SizePotion::OnStart() {
		auto caster = GetCaster();
        if (!caster) {
			return;
		}
        auto& runtime = Runtime::GetSingleton();
        auto saved_data = Persistent::GetSingleton().GetActorData(caster);
        float PotionPower = this->Strenght;
        float BonusSize = runtime.sizeLimit->value * PotionPower;
        saved_data->bonus_max_size = BonusSize;
        
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
        saved_data->bonus_max_size = 0;
	}
}