#include "managers/cameras/fp/combat.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "scale/height.hpp"

using namespace RE;

namespace Gts {
	NiPoint3 FirstPersonCombat::GetPlayerLocalOffset(const NiPoint3& cameraPos) {
		auto player = PlayerCharacter::GetSingleton();
		if (player) {
			float scale = get_visual_scale(player);
			float base_height = get_base_height(player);
			log::info("Combat Player Local Offset");
			return NiPoint3(0.0, 0.0, -base_height*(scale-1.0));
		}
		return NiPoint3();
	}
}
