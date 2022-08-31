#include "magic/effects/GrowthPotion.hpp"
#include "magic/effects/common.hpp"
#include "managers/GrowthTremorManager.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"

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
		const float BASE_POWER = 0.00150;
		const float DUAL_CAST_BONUS = 2.0;
		auto& runtime = Runtime::GetSingleton();
        float delta_time = *g_delta_time;

		auto caster = GetCaster();
		if (!caster) {
			return;
		}

        if (caster == PlayerCharacter::GetSingleton()) {
            GrowthTremorManager::GetSingleton().CallRumble(caster, caster, 0.5);
        
        }
        if (this->SoundTick <= 0.0)
		{
		    auto GrowthSound = runtime.growthSound;
		    float Volume = clamp(0.25, 2.0, get_visual_scale(caster));
		    PlaySound(GrowthSound, caster, Volume, 0.0);
		}
		this->SoundTick +=delta_time;

		if (this->SoundTick >= 160.0)
		{this->SoundTick = 0.0;}
        float Power = BASE_POWER;

		Grow(caster, 0.0, power);
	}
}