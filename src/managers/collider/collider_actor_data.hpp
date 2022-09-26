#pragma once
// Module that handles footsteps
#include <atomic>
#include "managers/collider/charcont_data.hpp"
#include "managers/collider/ragdoll_data.hpp"


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

		private:
			mutable std::mutex _lock;

			float last_scale = -1.0;

			std::atomic_uint64_t last_update_frame = std::atomic_uint64_t(0);

			RagdollData ragdollData;
			CharContData charContData;
	};
}
