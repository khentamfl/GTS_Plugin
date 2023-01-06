#include "managers/cameras/tp/foot.hpp"
#include "managers/cameras/camutil.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "node.hpp"
#include "util.hpp"

using namespace RE;

const float CAMERA_FACTOR = 95.00;
const float CAMERA_ZOOM = -30.0;
const float CAMERA_SIDE = 30.0;

namespace {

	NiMatrix3 QuatToMatrix(const NiQuaternion& q){
		float sqw = q.w*q.w;
		float sqx = q.x*q.x;
		float sqy = q.y*q.y;
		float sqz = q.z*q.z;

		// invs (inverse square length) is only required if quaternion is not already normalised
		float invs = 1 / (sqx + sqy + sqz + sqw);
		float m00 = ( sqx - sqy - sqz + sqw)*invs; // since sqw + sqx + sqy + sqz =1/invs*invs
		float m11 = (-sqx + sqy - sqz + sqw)*invs;
		float m22 = (-sqx - sqy + sqz + sqw)*invs;

		float tmp1 = q.x*q.y;
		float tmp2 = q.z*q.w;
		float m10 = 2.0 * (tmp1 + tmp2)*invs;
		float m01 = 2.0 * (tmp1 - tmp2)*invs;

		tmp1 = q.x*q.z;
		tmp2 = q.y*q.w;
		float m20 = 2.0 * (tmp1 - tmp2)*invs;
		float m02 = 2.0 * (tmp1 + tmp2)*invs;
		tmp1 = q.y*q.z;
		tmp2 = q.x*q.w;
		float m21 = 2.0 * (tmp1 + tmp2)*invs;
		float m12 = 2.0 * (tmp1 - tmp2)*invs;

		return NiMatrix3(
			NiPoint3(m00, m01, m02),
			NiPoint3(m10, m11, m12),
			NiPoint3(m20, m21, m22)
			);
	}

	NiPoint3 CompuleLookAt() {
		auto camera = PlayerCamera::GetSingleton();
		auto camState = camera->cameraStates[CameraState::kFirstPerson].get();
		NiPoint3 cameraTrans;
		camState->GetTranslation(cameraTrans);
		return cameraTrans;
	}
}

namespace Gts {
	void Foot::EnterState() {
		this->smoothIn.value = 0.0;
		this->smoothIn.target = 1.0;
		this->smoothIn.velocity = 0.0;
	}

	NiPoint3 Foot::GetOffset(const NiPoint3& cameraPos) {
		return NiPoint3();
	}

	NiPoint3 Foot::GetCombatOffset(const NiPoint3& cameraPos) {
		return NiPoint3();
	}

	NiPoint3 Foot::GetPlayerLocalOffset(const NiPoint3& cameraPos) {
		NiPoint3 footPos = this->GetFootPos();
		return footPos;
	}

	NiPoint3 Foot::GetPlayerLocalOffsetInstant() {
		auto player = PlayerCharacter::GetSingleton();
		float playerScale = get_visual_scale(player);

		NiPoint3 lookAt = CompuleLookAt();

		if (player) {
			auto rootModel = player->Get3D(false);
			if (rootModel) {
				auto transform = rootModel->world.Invert();
				auto localLookAt = transform*lookAt;
				return -this->smoothIn.value * localLookAt;
			}
		}
		return NiPoint3()
	}

	NiPoint3 Foot::GetFootPos() {
		const std::string_view leftFootLookup = "NPC L Foot [Lft ]";
		const std::string_view rightFootLookup = "NPC R Foot [Rft ]";
		auto player = PlayerCharacter::GetSingleton();
		if (player) {
			auto rootModel = player->Get3D(false);
			if (rootModel) {
				auto transform = rootModel->world.Invert();
				auto leftFoot = find_node(player, leftFootLookup);
				auto rightFoot = find_node(player, rightFootLookup);
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
