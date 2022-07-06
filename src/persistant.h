#pragma once
// Module that holds data that is persistant across saves
#include <SKSE/SKSE.h>

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	struct ActorData {
		float native_scale;
		float visual_scale;
		float visual_scale_v;
		float target_scale;
		float max_scale;
	};
	class Persistant {
		public:
			[[nodiscard]] static Persistant& GetSingleton() noexcept;
			static void OnRevert(SKSE::SerializationInterface*);
			static void OnGameSaved(SKSE::SerializationInterface* serde);
			static void OnGameLoaded(SKSE::SerializationInterface* serde);

			ActorData* GetActorData(Actor* actor);
		private:
			Persistant() = default;

			mutable std::mutex _lock;
			std::unordered_map<RE::Actor*, ActorData> _actor_data;
	};
}
