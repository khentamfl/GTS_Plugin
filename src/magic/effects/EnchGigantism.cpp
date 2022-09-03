#include "magic/effects/EnchGigantism.hpp"
#include "magic/effects/common.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"
#include "util.hpp"


namespace Gts {
	std::string Gigantism::GetName() {
		return "Gigantism";
	}

	bool Gigantism::StartEffect(EffectSetting* effect) { // NOLINT
		auto& runtime = Runtime::GetSingleton();
		return effect == runtime.EnchGigantism;
	}

	void Gigantism::OnStart() {
		auto CrushedFoe = GetTarget();
		this->CrushGrowthAmount += 1.0;
		this->ScaleOnCrush = get_visual_scale(CrushedFoe);
	}

	void Gigantism::OnUpdate() {
		auto& runtime = Runtime::GetSingleton();
		auto caster = GetCaster();

		if (!caster) {
			return;
		}
        float GigantismPower = GetActiveEffect()->magnitude;
        Persistent::GetSingleton().GetActorData(caster)->gigantism_enchantment = GigantismPower;
		log::info("GigantismPower is {}", GigantismPower);
	}


    void Gigantism::OnFinish() {
        this->CrushGrowthAmount = 0.0;
		this->ScaleOnCrush = 1.0;
    }
}