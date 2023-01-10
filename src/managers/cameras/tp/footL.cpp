#include "managers/cameras/tp/footL.hpp"
#include "managers/cameras/camutil.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "node.hpp"
#include "util.hpp"

using namespace RE;

namespace {
	const float OFFSET = 0.04f * 70.0f; // About 4cm
}

namespace Gts {
	NiPoint3 FootL::GetFootPos() {
		const std::string_view leftFootLookup = "NPC L Foot [Lft ]";
		auto player = PlayerCharacter::GetSingleton();
		if (player) {
			auto rootModel = player->Get3D(false);
			if (rootModel) {
				auto transform = rootModel->world.Invert();
				auto leftFoot = find_node(player, leftFootLookup);
				if (leftFoot != nullptr) {
					float playerScale = get_visual_scale(player);
					auto leftPosLocal = transform * (leftFoot->world * NiPoint3());
					this->smoothFootPos.target = leftPosLocal;
					this->smoothFootPos.target.z -= OFFSET*playerScale;
				}
			}
		}
		return this->smoothFootPos.value;
	}
}
