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
		float Distance = get_distance_to_camera(Source);
		float SourceSize = get_target_scale(Source);
		float ReceiverSize = get_target_scale(Receiver);
		float SizeDifference = clamp(0.0, 10.0, SourceSize/ReceiverSize);
		float falloff = 450 * (SourceSize * 0.25 + 0.75) * (SizeDifference * 0.25 + 0.75);
		float power = (0.425 * ShakeStrength(Source));
		float duration = 0.25 * (1 + (SizeDifference * 0.25));
		if (Distance < falloff) {
			float intensity = ((falloff/Distance) / 8);
			intensity = intensity*power;
			duration = duration * intensity;

			if (intensity <= 0) {
				intensity = 0;
			}
			if (power >= 12.6) {
				power = 12.6;
			}
			if (duration > 1.2) {
				duration = 1.2;
			}
			//log::info("Shake, Source: {}, Receiver: {}, Intensity: {}, Distance: {}, Falloff: {}", Source->GetDisplayFullName(), Receiver->GetDisplayFullName(), intensity, Distance, falloff);

			if (Receiver == Player) {
				shake_controller(intensity*Modifier, intensity*Modifier, duration);
				shake_camera(Receiver, intensity*Modifier, duration);
			}
		}
	}

	inline float GrowthTremorManager::ShakeStrength(Actor* Source) {
		float Size = get_visual_scale(Source);
		float k = 0.065;
		float n = 1.0;
		float s = 1.12;
		float Result = 1.0/(pow(1.0+pow(k*(Size-1.0),n*s),1.0/s));
		return Result;
	}
}
