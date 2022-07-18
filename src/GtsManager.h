#pragma once
// Module for the Gts Related code
#include <vector>
#include <atomic>
#include <unordered_map>

#include <RE/Skyrim.h>

#include "scale.h"
#include "node.h"
#include "util.h"

using namespace std;
using namespace RE;

namespace Gts {
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
			bool enabled = false;

			void poll();
			void poll_actor(Actor* actor);

			// Reapply changes (used after reload events)
			void reapply();
			void reapply_actor(Actor* actor);
	};
}
