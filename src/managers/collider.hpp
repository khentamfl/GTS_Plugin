#pragma once
// Module that handles footsteps


using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	struct CapsuleData {
		hkVector4 start;
		hkVector4 end;
		float radius;
	};

	class ActorData {
		public:
			CapsuleData* GetCapsuleData(hkpCapsuleShape* capsule);

		private:
			mutable std::mutex _lock;
			std::unordered_map<hkpCapsuleShape*, CapsuleData> capsule_data;
	};

	class ColliderManager {
		public:
			[[nodiscard]] static ColliderManager& GetSingleton() noexcept;

			void Update();

			ActorData* GetActorData(Actor* actor);
		private:
			mutable std::mutex _lock;
			std::unordered_map<Actor*, ActorData > actor_data;
	};
}
