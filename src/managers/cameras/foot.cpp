#include "managers/cameras/foot.hpp"
#include "managers/cameras/util.hpp"
#include "data/runtime.hpp"
#include "node.hpp"

using namespace RE;

const float CAMERA_FACTOR = 0.75;

namespace Gts {
	NiPoint3 Foot::GetOffset(const NiPoint3& cameraPos) {
		float lookY = GetINIFloat("fOverShoulderPosY:Camera");

		return -(cameraPos*CAMERA_FACTOR) - NiPoint3(0.0, lookY, 0.0)*CAMERA_FACTOR + GetFootPos() + NiPoint3(
			Runtime::GetFloat("cameraAlternateX"),
			0.0,
			Runtime::GetFloat("cameraAlternateY")
			);
	}

	NiPoint3 Foot::GetCombatOffset(const NiPoint3& cameraPos) {
		float lookY = GetINIFloat("fOverShoulderPosY:Camera");

		return -(cameraPos*CAMERA_FACTOR) - NiPoint3(0.0, lookY, 0.0)*CAMERA_FACTOR + GetFootPos() + NiPoint3(
			Runtime::GetFloat("combatCameraAlternateX"),
			0.0,
			Runtime::GetFloat("combatCameraAlternateY")
			);
	}

	NiPoint3 Foot::GetFootPos() {
		const std::string_view leftFootRegex = ".*(L.*Foot|L.*Leg.*Tip).*";
		const std::string_view rightFootRegex = ".*(R.*Foot|R.*Leg.*Tip).*";
		auto player = PlayerCharacter::GetSingleton();
		if (player) {
			auto rootModel = player->Get3D(false);
			if (rootModel) {
				auto transform = rootModel->world.Invert();
				auto leftFoot = find_node_regex(player, leftFootRegex);
				auto rightFoot = find_node_regex(player, rightFootRegex);
				if (leftFoot != nullptr && rightFoot != nullptr) {
					auto leftPosLocal = transform * (leftFoot->world * NiPoint3());
					auto rightPosLocal = transform * (rightFoot->world * NiPoint3());
					this->smoothFootPos.target = (leftPosLocal + rightPosLocal) / 2.0;
				}
			}
		}
		return this->smoothFootPos.value;
	}
}
