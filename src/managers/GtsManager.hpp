#pragma once
// Module for the Gts Related code
#include <vector>
#include <atomic>
#include <unordered_map>

#include <RE/Skyrim.h>

#include "events.hpp"
#include "node.hpp"

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

			float experiment = 1.0;
			inline static bool balancemode = false;

			virtual std::string DebugName() override;
			virtual void Update() override;

			// Reapply changes (used after reload events)
			void reapply(bool force = true);
			void reapply_actor(Actor* actor, bool force = true);
		private:
			SoftPotential speedadjustment { 
				.k = 0.142, // 0.125
				.n = 0.82, // 0.86
				.s = 1.90, // 1.12
				.o = 1.0,
				.a = 0.0,  //Default is 0
		};
	};
}
