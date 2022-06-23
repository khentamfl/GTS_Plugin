#pragma once

#include <RE/Skyrim.h>

namespace Gts {

/**
 * The class which tracks gts size effects.
 */
	class GtsManager {
public:
	std::atomic_bool aborted;

/**
 * Get the singleton instance of the <code>GtsManager</code>.
 */
	[[nodiscard]] static GtsManager& GetSingleton() noexcept;

	void poll();

	void loop();

	// Stop looping
	void abort();
	};
}
