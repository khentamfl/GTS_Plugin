#pragma once
// Module that handles footsteps
#include <SKSE/SKSE.h>
#include "managers/impact.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	class FootStepManager {
		public:
			[[nodiscard]] static FootStepManager& GetSingleton() noexcept;

			void OnImpact(const Impact& impact);
	};
}
