#pragma once
// Module that handles footsteps


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

	class ActorData {
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

			inline std::unordered_map<const hkpCapsuleShape*, CapsuleData>& GetCapsulesData() {
				return this->capsule_data;
			}

			float last_scale = -1.0;
		private:
			mutable std::mutex _lock;
			std::unordered_map<const hkpCapsuleShape*, CapsuleData> capsule_data;
	};

	class ColliderManager {
		public:
			[[nodiscard]] static ColliderManager& GetSingleton() noexcept;

			void Update();
			inoine void FlagReset() {
				this->reset.store(true);
			}

			ActorData* GetActorData(Actor* actor);
		private:
			mutable std::mutex _lock;
			std::unordered_map<Actor*, ActorData > actor_data;
			static std::atomic_bool reset = false;
	};
}
