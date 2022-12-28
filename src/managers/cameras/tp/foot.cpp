#include "managers/cameras/tp/foot.hpp"
#include "managers/cameras/camutil.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "node.hpp"
#include "util.hpp"

using namespace RE;

const float CAMERA_FACTOR = 90.00;

namespace Gts {
	NiPoint3 Foot::GetOffset(const NiPoint3& cameraPos) {
		return NiPoint3();
	}

	NiPoint3 Foot::GetCombatOffset(const NiPoint3& cameraPos) {
		return NiPoint3();
	}

	NiPoint3 Foot::GetPlayerLocalOffset(const NiPoint3& cameraPos) {
		NiPoint3 footPos = this->GetFootPos();
		auto player = PlayerCharacter::GetSingleton();
		float playerScale = get_visual_scale(player);

		return footPos - NiPoint3(0.0, 0.0, CAMERA_FACTOR*playerScale);
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
