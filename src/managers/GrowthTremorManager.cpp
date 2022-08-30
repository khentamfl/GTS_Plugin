#include "managers/camera.hpp"
#include "managers/GtsManager.hpp"
#include "managers/GrowthTremorManager.hpp"
#include "scale/scale.hpp"
#include "util.hpp"
#include "data/runtime.hpp"
#include "data/persistent.hpp"
#include "data/transient.hpp"
#include "magic/effects/common.hpp"

namespace Gts {
	GrowthTremorManager& GrowthTremorManager::GetSingleton() noexcept {
		static GrowthTremorManager instance;
		return instance;
	}

	void GrowthTremorManager::CallRumble(Actor* Source, Actor* Receiver, float Modifier) {
        auto Player = PlayerCharacter::GetSingleton();
        float Distance = get_distance_to_camera(Receiver); // No way to call Source->GetDistance like in SP, so i'll have to use Camera
        float SourceSize = get_visual_scale(Source);
        float ReceiverSize = get_visual_scale(Receiver);
        float SizeDifference = SourceSize/ReceiverSize;
        float falloff = 250 * (SourceSize * 0.75 + 0.25);
        float power = (0.425 / ShakeStrength(Source) * (SizeDifference * 0.25 + 1.0));
        float duration = (0.15, 1.2, 0.25 * (1 + (size * 0.25)));
         if (Distance < falloff)
            {
             float intensity = ((falloff/playerDist) / 8);
             intensity = intensity*power*Modifier;
             duration = duration * intensity;

            if (intensity <= 0)
            {
                 intensity = 0;
            }
            if (power >= 12.6)
            {
             power = 12.6;
            }
            if (duration > 1.2)
             {
                    duration = 1.2;
             }
        shake_camera(Receiver, intensity, duration)
        if (Receiver == Player) {
        shake_controller(intensity, intensity, duration)
            }
        }
    }

    void GrowthTremorManager::ShakeStrength(Actor* Source) {
        float Size = get_visual_scale(Source);
        float k = 0.065; 
        float n = 1.0; 
        float n = 1.12;
        float Result = 1.0/(pow(1.0+pow(k*(Size-1.0),n*s),1.0/s)); 
        return Result;
    }
}

