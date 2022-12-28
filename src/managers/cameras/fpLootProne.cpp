#include "managers/cameras/fpLootProne.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"

using namespace RE;

namespace {
	const float BASE_HEIGHT = 170.0;

	NiPoint3 ProneAdjustment(const NiPoint3& cameraPos) {
		return clamp(0.25, 20.0, 3.0 * Runtime::GetFloat("ProneOffsetFP"));
	}
}

namespace Gts {
	NiPoint3 FirstPersonLootProne::GetPlayerLocalOffset(const NiPoint3& cameraPos) {
		auto player = PlayerCharacter::GetSingleton();
		if (player) {
			float scale = get_visual_scale(player);
			return NiPoint3(0.0, 0.0, -BASE_HEIGHT*(scale-0.7)*(1.0-ProneAdjustment(cameraPos)));
		}
		return NiPoint3();
	}

	bool FirstPersonLootProne::PermitManualEdit() {
		return false;
	}
}
