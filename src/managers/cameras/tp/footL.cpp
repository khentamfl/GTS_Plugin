#include "managers/cameras/tp/footL.hpp"
#include "managers/cameras/camutil.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "node.hpp"
#include "util.hpp"

using namespace RE;

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
					auto leftPosLocal = transform * (leftFoot->world * NiPoint3());
					this->smoothFootPos.target = leftPosLocal;
				}
			}
		}
		return this->smoothFootPos.value;
	}
}
