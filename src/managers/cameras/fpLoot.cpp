#include "managers/cameras/fpLoot.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"

using namespace RE;

namespace {
	const float BASE_HEIGHT = 170.0;
}

namespace Gts {
	NiPoint3 FirstPersonLoot::GetPlayerLocalOffset(const NiPoint3& cameraPos) {
		auto player = PlayerCharacter::GetSingleton();
		if (player) {
			float scale = get_visual_scale(player);
			return NiPoint3(0.0, 0.0, -BASE_HEIGHT*(scale-0.7));
		}
		return NiPoint3();
	}

	bool FirstPersonLoot::PermitManualEdit() {
		return false;
	}
}
