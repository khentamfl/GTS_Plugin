#include "managers/cameras/tp/footR.hpp"
#include "managers/cameras/camutil.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "node.hpp"
#include "util.hpp"

using namespace RE;

namespace Gts {
	void FootR::EnterState() {
		auto player = PlayerCharacter::GetSingleton();
		if (player) {
			float playerScale = get_visual_scale(player);
			if (playerScale > 0.0) {
				this->smoothScale.value = playerScale;
				this->smoothScale.target = playerScale;
				this->smoothScale.velocity = 0.0;
			}
		}
	}

	NiPoint3 FootR::GetOffset(const NiPoint3& cameraPos) {
		return NiPoint3();
	}

	NiPoint3 FootR::GetCombatOffset(const NiPoint3& cameraPos) {
		return NiPoint3();
	}

	NiPoint3 FootR::GetPlayerLocalOffset(const NiPoint3& cameraPos) {
		NiPoint3 footPos = this->GetFootPos();
		return footPos;
	}

	NiPoint3 FootR::GetPlayerLocalOffsetInstant() {
		auto player = PlayerCharacter::GetSingleton();
		float playerScale = get_visual_scale(player);

		NiPoint3 lookAt = CompuleLookAt();

		if (player) {
			auto rootModel = player->Get3D(false);
			if (rootModel) {
				auto transform = rootModel->world.Invert();
				NiPoint3 localLookAt = transform*lookAt;
				this->smoothScale.target = playerScale;
				return localLookAt * -1 * this->smoothScale.value;
			}
		}
		return NiPoint3();
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
