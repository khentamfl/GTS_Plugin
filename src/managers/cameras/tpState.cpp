#include "managers/cameras/tpState.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "node.hpp"

using namespace RE;
using namespace Gts;

namespace Gts {
	NiPoint3 ThirdPersonCameraState::GetPlayerLocalOffset(const NiPoint3& cameraPos) {
		NiPoint3 pos = NiPoint3();
		auto player = PlayerCharacter::GetSingleton();
		if (player) {
			auto scale = get_visual_scale(player);
			auto bones = this->GetBoneTargets();
			auto bone_count = bones.size();
			if (bone_count > 0 ) {
				auto player = PlayerCharacter::GetSingleton();
				if (player) {
					auto rootModel = player->Get3D(false);
					if (rootModel) {
						auto transform = rootModel->world.Invert();

						NiPoint3 lookAt = CompuleLookAt();
						NiPoint3 localLookAt = transform*lookAt;
						this->smoothScale.target = scale;
						pos += localLookAt * -1 * this->smoothScale.value;

						NiPoint3 bonePos = NiPoint3();
						for (auto bone_name: bones) {
							auto node = find_node(player, bone_name);
							if (node) {
								auto localPos = transform * (node->world * NiPoint3());
								bonePos += localPos * (1.0/bone_count);
							} else {
								log::error("Bone not found for camera target: {}", bone_name);
							}
						}
						smoothedBonePos.target = bonePos;
						pos += smoothedBonePos.value;
					}
				}
			}
		}
		return pos;
	}

	NiPoint3 ThirdPersonCameraState::GetPlayerLocalOffsetProne(const NiPoint3& cameraPos) {
		NiPoint3 pos = this->GetPlayerLocalOffset(cameraPos);
		auto player = PlayerCharacter::GetSingleton();
		if (player) {
			auto scale = get_visual_scale(player);
			pos += this->ProneAdjustment(cameraPos)*scale;
		}
		return pos;
	}

	std::vector<std::string> ThirdPersonCameraState::GetBoneTargets() {
		return {};
	}

	NiPoint3 ThirdPersonCameraState::ProneAdjustment(const NiPoint3& cameraPos) {
		float proneFactor = 1.0 - Runtime::GetFloat("CalcProne");
		NiPoint3 result = NiPoint3();

		result.z = -cameraPos.z * proneFactor;
		return result;
	}
}
