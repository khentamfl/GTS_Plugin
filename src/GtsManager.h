#pragma once

#include <vector>
#include <atomic>
#include <unordered_map>

#include <RE/Skyrim.h>

using namespace std;
using namespace RE;

namespace Gts {

	enum SizeMethod {
		ModelScale = 0,
		RootScale = 1,
		RefScale = 2,
	};

	struct CachedBound {
		string name;
		float center[3];
		float extents[3];
	};

	void cache_bound(BSBound* src, CachedBound* dst);
	void uncache_bound(CachedBound* src, BSBound* dst);

	struct BaseHeight {
		CachedBound collisionBound;
		CachedBound bumperCollisionBound;
		float actorHeight;
		float swimFloatHeightRatio;
		NiTransform bumper_transform;
	};

	struct ActorExtraData {
		BaseHeight base_height;
		bool initialised;
	};
	/**
	 * The class which tracks gts size effects.
	 */
	class GtsManager {
		public:

			/**
			 * Get the singleton instance of the <code>GtsManager</code>.
			 */
			[[nodiscard]] static GtsManager& GetSingleton() noexcept;

			atomic_ulong frame_count = atomic_int64_t(0);
			unordered_map<FormID, ActorExtraData> actor_data;
			float test_scale;
			SizeMethod size_method;
			BSWin32KeyboardDevice* keyboard;


			void poll();
			void poll_actor(Actor* actor);
			ActorExtraData* get_actor_extra_data(Actor* actor);
			BSWin32KeyboardDevice* get_keyboard();

	};
}
