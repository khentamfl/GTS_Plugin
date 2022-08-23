#pragma once
// Module that handles footsteps
#include <atomic>
#include "managers/GtsManager.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	struct CapsuleData {
		unique_ptr<hkpCapsuleShape> capsule;
		hkVector4 start;
		hkVector4 end;
		float radius;
		CapsuleData(const hkpCapsuleShape* orig_capsule);
		CapsuleData(CapsuleData&& old) : capsule(std::move(old.capsule)), start(std::move(old.start)), end(std::move(old.end)), radius(std::move(old.radius)) {
		};
	};

	class ColliderActorData {
		public:
			inline CapsuleData* GetCapsuleData(hkpCapsuleShape* capsule) {
				CapsuleData* result = nullptr;
				auto key = capsule;
				try {
					auto result = &this->capsule_data.at(key);
				} catch (const std::out_of_range& oor) {
					result = nullptr;
				}
				return result;
			}

			void ReplaceCapsule(hkpRigidBody* rigid_body, const hkpCapsuleShape* orig_capsule);

			inline bool HasCapsuleData() {
				return !this->capsule_data.empty();
			}

			inline void Reset() {
				auto itr = this->capsule_data.begin();
				while (itr != this->capsule_data.end())
				{
					if (itr->second.capsule->GetReferenceCount() == 1) { // We are the only ref
						itr = this->capsule_data.erase(itr); // So lets delete
					} else {
						itr++;
					}
				}

				this->last_scale = -1.0;
				this->last_update_frame.store(0);
			}

			inline std::unordered_map<const hkpCapsuleShape*, CapsuleData>& GetCapsulesData() {
				return this->capsule_data;
			}

			float last_scale = -1.0;
			std::atomic_uint64_t last_update_frame = std::atomic_uint64_t(0);
		private:
			mutable std::mutex _lock;
			std::unordered_map<const hkpCapsuleShape*, CapsuleData> capsule_data;
	};

	class ColliderManager {
		public:
			[[nodiscard]] static ColliderManager& GetSingleton() noexcept;

			void Update();
			inline void FlagReset() {
				this->last_reset_frame.store(GtsManager::GetSingleton().GetFrameNum());
			}

			ColliderActorData* GetActorData(Actor* actor);
		private:
			mutable std::mutex _lock;
			std::unordered_map<Actor*, ColliderActorData > actor_data;
			std::atomic_uint64_t last_reset_frame = std::atomic_uint64_t(0);
	};
}
