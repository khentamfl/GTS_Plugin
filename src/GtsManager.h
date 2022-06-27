#pragma once

#include <vector>
#include <atomic>
#include <map>

#include <RE/Skyrim.h>

using namespace std;
using namespace RE;

namespace Gts {

	struct BaseHeight {
		BSBound collisionBound;
		BSBound bumperCollisionBound;
		float swimFloatHeight;
		float actorHeight;
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
			BSTHashMap<ObjectRefHandle, BaseHeight> base_heights;

			void poll();
			void poll_actor(Actor* actor);
			BaseHeight* get_base_height(Actor* actor);
	};
}
