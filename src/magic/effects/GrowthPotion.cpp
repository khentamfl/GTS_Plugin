#include "managers/GrowthTremorManager.hpp"
#include "managers/GtsManager.hpp"
#include "magic/effects/GrowthPotion.hpp"
#include "magic/effects/common.hpp"
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
		const float BASE_POWER = 0.00025;
		auto& runtime = Runtime::GetSingleton();
        float delta_time = *g_delta_time;
        float GrowthTickIs = this->SoundTick;

		auto caster = GetCaster();
		if (!caster) {
			return;
		}

        if (caster == PlayerCharacter::GetSingleton()) {
            GrowthTremorManager::GetSingleton().CallRumble(caster, caster, 0.4);
        
        }
        if (GtsManager::GetSingleton().GetFrameNum() % 120 * delta_time)
		{
		    auto GrowthSound = runtime.growthSound;
		    float Volume = clamp(0.25, 2.0, get_visual_scale(caster)/2);
		    PlaySound(GrowthSound, caster, Volume, 0.0);
		}
		
        float Power = BASE_POWER;

		Grow(caster, 0.0, Power);
	}
}