#include "managers/cameras/tp/footL.hpp"
#include "managers/cameras/camutil.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "node.hpp"
#include "util.hpp"

using namespace RE;

const float CAMERA_FACTOR = 100.00;
const float CAMERA_SIDE = 30.0;
const float CAMERA_ZOOM = 50.0;

namespace Gts {
	NiPoint3 FootL::GetOffset(const NiPoint3& cameraPos) {
		return NiPoint3();
	}

	NiPoint3 FootL::GetCombatOffset(const NiPoint3& cameraPos) {
		return NiPoint3();
	}

	NiPoint3 FootL::GetPlayerLocalOffset(const NiPoint3& cameraPos) {
		NiPoint3 footPos = this->GetFootPos();
		auto player = PlayerCharacter::GetSingleton();
		float playerScale = get_visual_scale(player);

		return footPos - NiPoint3(CAMERA_SIDE*playerScale, CAMERA_ZOOM*playerScale, CAMERA_FACTOR*playerScale);
	}

	NiPoint3 FootL::GetFootPos() {
		const std::string_view leftFootRegex = ".*(L.*Foot|L.*Leg.*Tip).*";
		auto player = PlayerCharacter::GetSingleton();
		if (player) {
			auto rootModel = player->Get3D(false);
			if (rootModel) {
				auto transform = rootModel->world.Invert();
				auto leftFoot = find_node_regex(player, leftFootRegex);
				if (leftFoot != nullptr) {
					auto leftPosLocal = transform * (leftFoot->world * NiPoint3());
					this->smoothFootPos.target = leftPosLocal;
				}
			}
		}
		return this->smoothFootPos.value;
	}


}
