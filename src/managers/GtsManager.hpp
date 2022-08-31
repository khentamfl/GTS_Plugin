#pragma once
// Module for the Gts Related code
#include <vector>
#include <atomic>
#include <unordered_map>

#include <RE/Skyrim.h>

#include "node.hpp"
#include "util.hpp"

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
			float experiment = 1.0;

			void poll();

			void on_update();

			// Reapply changes (used after reload events)
			void reapply(bool force = true);
			void reapply_actor(Actor* actor, bool force = true);

			inline uint64_t GetFrameNum() {
				return this->frame_count.load() * TimeScale();
			}
	};
}
