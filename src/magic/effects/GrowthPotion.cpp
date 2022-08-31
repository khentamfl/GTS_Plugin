#include "managers/GrowthTremorManager.hpp"
#include "managers/GtsManager.hpp"
#include "magic/effects/GrowthPotion.hpp"
#include "magic/effects/common.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"
#include "util.hpp"

namespace Gts {
	std::string GrowthPotion::GetName() {
		return "GrowthPotion";
	}

	bool GrowthPotion::StartEffect(EffectSetting* effect) { // NOLINT
		auto& runtime = Runtime::GetSingleton();
		return effect == runtime.EffectGrowthPotion;

	}

    void GrowthPotion::OnStart() {
        auto player = PlayerCharacter::GetSingleton();
        GrowthTremorManager::GetSingleton().CallRumble(player, player, 2.0);
    }

	void GrowthPotion::OnUpdate() {
		const float BASE_POWER = 0.000040;
		auto& runtime = Runtime::GetSingleton();
        

		auto caster = GetCaster();
		if (!caster) {
			return;
		}
        float AlchemyLevel = clamp(1.0, 2.0, caster->GetActorValue(ActorValue::kAlchemy)/100);

        if (caster == PlayerCharacter::GetSingleton()) {
            GrowthTremorManager::GetSingleton().CallRumble(caster, caster, 0.4);
        
        }
        if (GtsManager::GetSingleton().GetFrameNum() % 120 * TimeScale() == 0)
		{
		    auto GrowthSound = runtime.growthSound;
		    float Volume = clamp(0.25, 2.0, get_visual_scale(caster)/2);
		    PlaySound(GrowthSound, caster, Volume, 0.0);
		}
		
        float Power = BASE_POWER * AlchemyLevel;

		Grow(caster, 0.0, Power);
	}
}