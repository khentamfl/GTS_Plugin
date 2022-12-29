#include "managers/cameras/tp/footR.hpp"
#include "managers/cameras/camutil.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "node.hpp"
#include "util.hpp"

using namespace RE;

const float CAMERA_FACTOR = 130.00;
const float CAMERA_SIDE = 30.0;
const float CAMERA_ZOOM = -90.0;

namespace Gts {
	NiPoint3 FootR::GetOffset(const NiPoint3& cameraPos) {
		return NiPoint3();
	}

	NiPoint3 FootR::GetCombatOffset(const NiPoint3& cameraPos) {
		return NiPoint3();
	}

	NiPoint3 FootR::GetPlayerLocalOffset(const NiPoint3& cameraPos) {
		NiPoint3 footPos = this->GetFootPos();
		auto player = PlayerCharacter::GetSingleton();
		float playerScale = get_target_scale(player);

		return footPos - NiPoint3(CAMERA_SIDE*playerScale, CAMERA_ZOOM*playerScale, CAMERA_FACTOR*playerScale);
	}

	NiPoint3 FootR::GetFootPos() {
		const std::string_view rightFootLookup = "NPC R Foot [Rft ]";
		auto player = PlayerCharacter::GetSingleton();
		if (player) {
			auto rootModel = player->Get3D(false);
			if (rootModel) {
				auto transform = rootModel->world.Invert();
				auto rightFoot = find_node(player, rightFootLookup);
				if (rightFoot != nullptr) {
					auto rightPosLocal = transform * (rightFoot->world * NiPoint3());
					this->smoothFootPos.target = rightPosLocal;
				}
			}
		}
		return this->smoothFootPos.value;
	}


}
