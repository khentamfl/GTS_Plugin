#pragma once

#include <vector>

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

			void poll();
	};
}
