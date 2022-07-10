#pragma once
// Module that holds data that is persistent across saves
#include <SKSE/SKSE.h>
#include "scale.h"

using namespace std;
using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace Gts {
	struct ActorData {
		float native_scale;
		float visual_scale;
		float visual_scale_v;
		float target_scale;
		float max_scale;
		float half_life;
		float anim_speed;
	};
	class Persistent {
		public:
			[[nodiscard]] static Persistent& GetSingleton() noexcept;
			static void OnRevert(SKSE::SerializationInterface*);
			static void OnGameSaved(SKSE::SerializationInterface* serde);
			static void OnGameLoaded(SKSE::SerializationInterface* serde);

			ActorData* GetActorData(Actor* actor);

			bool highheel_correction = true;
			SizeMethod size_method = SizeMethod::ModelScale;
		private:
			Persistent() = default;

			mutable std::mutex _lock;
			std::unordered_map<FormID, ActorData> _actor_data;
	};
}
