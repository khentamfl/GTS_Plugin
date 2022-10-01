#pragma once
// Module that holds data that is persistent across saves

#include "events.hpp"
#include "scale/modscale.hpp"
#include "util.hpp"

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
		float target_scale_v;
		float max_scale;
		float half_life;
		float anim_speed;
		float effective_multi;
		float bonus_hp;
		float bonus_carry;
		float bonus_max_size;
		float smt_run_speed;

		ActorData();
		ActorData(Actor* actor);
	};

	struct CameraCollisions {
		bool enable_trees = false;
		bool enable_debris = false;
		bool enable_static = true;
		bool enable_terrain = true;
		bool enable_actor = false;
		float above_scale = 5.0;
	};

	class Persistent : public Gts::EventListener {
		public:
			virtual void Reset() override;
			virtual void ResetActor(Actor* actor) override;

			[[nodiscard]] static Persistent& GetSingleton() noexcept;
			static void OnRevert(SKSE::SerializationInterface*);
			static void OnGameSaved(SKSE::SerializationInterface* serde);
			static void OnGameLoaded(SKSE::SerializationInterface* serde);

			ActorData* GetActorData(Actor* actor);
			ActorData* GetData(TESObjectREFR* refr);



			bool highheel_correction = true;
			bool is_speed_adjusted = true;
			float tremor_scale = 1.0;
			float npc_tremor_scale = 1.0;
			SoftPotential speed_adjustment {
				.k = 0.125, // 0.125
				.n = 0.20, // 0.86
				.s = 1.12,
				.o = 1.0,
				.a = -0.0,  //Default is 0
			};
			SoftPotential MS_adjustment {
				.k = 0.132, // 0.132
				.n = 0.20, // 0.86
				.s = 1.12,
				.o = 1.0,
				.a = 0.0, //Default is 0
			};
			SizeMethod size_method = SizeMethod::ModelScale;
			CameraCollisions camera_collisions;
		private:
			Persistent() = default;

			mutable std::mutex _lock;
			std::unordered_map<FormID, ActorData> _actor_data;
	};
}
