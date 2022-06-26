#pragma once

#include <vector>
#include <atomic>

#include <RE/Skyrim.h>

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

			std::atomic_ulong frame_count = std::atomic_int64_t(0);

			void poll();
			void poll_actor(RE::Actor* actor);
	};
}
