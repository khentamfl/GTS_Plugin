#pragma once
// Module that handles footsteps
#include <SKSE/SKSE.h>
#include "managers/impact.h"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	class TremorManager {
		public:
			[[nodiscard]] static TremorManager& GetSingleton() noexcept;

			void OnImpact(const Impact& impact);
	};
}
