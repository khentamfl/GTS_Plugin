#pragma once
// Module that handles footsteps
#include <atomic>
#include "hooks/RE.hpp"
#include "managers/collider/collider_actor_data.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	class ColliderManager {
		public:
			[[nodiscard]] static ColliderManager& GetSingleton() noexcept;

			void Update();
			void UpdateHavok();

			void FlagReset();

			void ResetActor(Actor* actor);

			ColliderActorData* GetActorData(Actor* actor);
		private:
			mutable std::mutex _lock;
			std::unordered_map<Actor*, ColliderActorData > actor_data;
			std::atomic_uint64_t last_reset_frame = std::atomic_uint64_t(0);
			TESObjectCELL* previous_cell = nullptr;
	};
}
