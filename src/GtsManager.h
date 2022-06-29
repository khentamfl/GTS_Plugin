#pragma once

#include <vector>
#include <atomic>
#include <unordered_map>

#include <RE/Skyrim.h>

using namespace std;
using namespace RE;

namespace Gts {

	struct CachedBound {
		string name;
		float center[3];
		float extents[3];
	};

	void cache_bound(BSBound* src, CachedBound* dst) {
		dst->center[0] = src->center.x;
		dst->center[1] = src->center.y;
		dst->center[2] = src->center.z;
		dst->extents[0] = src->extents.x;
		dst->extents[1] = src->extents.y;
		dst->extents[2] = src->extents.z;
	}
	void uncache_bound(CachedBound* src, BSBound* dst) {
		dst->center[0] = src->center.x;
		dst->center[1] = src->center.y;
		dst->center[2] = src->center.z;
		dst->extents[0] = src->extents.x;
		dst->extents[1] = src->extents.y;
		dst->extents[2] = src->extents.z;
	}

	struct BaseHeight {
		CachedBound collisionBound;
		CachedBound bumperCollisionBound;
		float swimFloatHeight;
		float actorHeight;
	};

	struct ActorExtraData {
		BaseHeight base_height;
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

			void poll();
			void poll_actor(Actor* actor);
	};
}
