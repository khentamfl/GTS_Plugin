#include "managers/cameras/fp/loot.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "scale/height.hpp"

using namespace RE;

namespace Gts {
	NiPoint3 FirstPersonLoot::GetPlayerLocalOffset(const NiPoint3& cameraPos) {
		auto player = PlayerCharacter::GetSingleton();
		if (player) {
			float scale = get_visual_scale(player);
			float base_height = get_base_height(player);
			return NiPoint3(0.0, 0.0, -base_height*(scale-0.7));
		}
		return NiPoint3();
	}
}
