#pragma once
// Module that handles footsteps
#include "managers/collider/capsule_data.hpp"
#include "managers/collider/rb_data.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {

	struct RagdollData {
		RagdollData(hkaRagdollInstance* ragdoll);
		RagdollData(RagdollData&& old) : ragdoll(std::move(old.ragdoll)), capsule_data(std::move(old.capsule_data)), rb_data(std::move(old.rb_data)) {
		};
		~RagdollData();
		void Init(hkaRagdollInstance* ragdoll);

		void ApplyScale(const float& new_scale, const hkVector4& vecScale);
		void ApplyPose(const hkVector4& origin, const float& new_scale);
		void PruneColliders(Actor* actor);
		void AddCapsule(hkpCapsuleShape* capsule);
		void AddRidgedBody(hkpRigidBody* rigid_body);

		hkaRagdollInstance* ragdoll = nullptr;
		std::unordered_map<hkpCapsuleShape*, CapsuleData> capsule_data;
		std::unordered_map<hkpRigidBody*, RbData> rb_data;

	};
}
