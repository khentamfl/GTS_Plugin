#include "managers/cameras/tp/footR.hpp"
#include "managers/cameras/camutil.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "node.hpp"
#include "util.hpp"

using namespace RE;

namespace {
	const float OFFSET = 0.18f * 70.0f; // About 18cm
}

namespace Gts {
	NiPoint3 FootR::GetFootPos() {
		float base_hh = 0;
		const std::string_view rightFootLookup = "NPC R Foot [Rft ]";
		auto player = PlayerCharacter::GetSingleton();
		NiAVObject* npc_node = find_node_any(player, "NPC");
			if (npc_node) {
				base_hh = npc_node->local.translate.z;
			}
			

		if (player) {
			auto rootModel = player->Get3D(false);
			if (rootModel) {
				auto transform = rootModel->world.Invert();
				auto rightFoot = find_node(player, rightFootLookup);
				if (rightFoot != nullptr) {
					float playerScale = get_visual_scale(player);
					auto rightPosLocal = transform * (rightFoot->world * NiPoint3());
					this->smoothFootPos.target = rightPosLocal;
					if (base_hh > 0.01) {
						this->smoothFootPos.target.z -= OFFSET*playerScale;
					}
				}
			}
		}
		return this->smoothFootPos.value;
	}


}
