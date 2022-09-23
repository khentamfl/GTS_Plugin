#pragma once
// Module that handles footsteps
#include <atomic>
#include "hooks/RE.hpp"
#include "managers/collider/capsule_data.hpp"
#include "managers/collider/convex_data.hpp"
#include "managers/collider/list_data.hpp"
#include "managers/collider/rb_data.hpp"



using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	class ColliderActorData {
		public:
			ColliderActorData(Actor* actor);
			~ColliderActorData();

			void FlagUpdate();
			void Reset();
			void Update(Actor* actor, std::uint64_t last_reset_frame);
			void ApplyScale(const float& new_scale, const hkVector4& vec_scale);
			void ApplyPose(Actor* actor, const float& scale);
			void UpdateColliders(Actor* actor);
			void PruneColliders(Actor* actor);
			void AddRagdoll(hkaRagdollInstance* ragdoll);
			void AddCharController(bhkCharacterController* controller);
			void AddShape(hkpShape* shape);
			void AddCapsule(hkpCapsuleShape* capsule);
			void AddConvexVerts(hkpConvexVerticesShape* convex);
			void AddList(hkpListShape* list);
			void AddRidgedBody(hkpRigidBody* rigid_body);

		private:
			mutable std::mutex _lock;

			float last_scale = -1.0;
			bhkCharacterController* lastCharController = nullptr;
			float charControllerCenter = 0.0;

			std::atomic_uint64_t last_update_frame = std::atomic_uint64_t(0);
			FormID form_id = 0;

			std::unordered_map<hkpCapsuleShape*, CapsuleData> capsule_data;
			std::unordered_map<hkpConvexVerticesShape*, ConvexData> convex_data;
			std::unordered_map<hkpListShape*, ListData> list_data;
			std::unordered_map<hkpRigidBody*, RbData> rb_data;


	};
}
