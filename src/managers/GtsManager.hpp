#pragma once
// Module for the Gts Related code
#include <vector>
#include <atomic>
#include <unordered_map>

#include <RE/Skyrim.h>

#include "events.hpp"
#include "node.hpp"
#include "util.hpp"

using namespace std;
using namespace RE;

namespace Gts {
	/**
	 * The class which tracks gts size effects.
	 */
	class GtsManager : public EventListener  {
		public:

			/**
			 * Get the singleton instance of the <code>GtsManager</code>.
			 */
			[[nodiscard]] static GtsManager& GetSingleton() noexcept;

			atomic_ulong frame_count = atomic_int64_t(0);
			float experiment = 1.0;

			virtual void Update() override;

			// Reapply changes (used after reload events)
			void reapply(bool force = true);
			void reapply_actor(Actor* actor, bool force = true);
	};
}
