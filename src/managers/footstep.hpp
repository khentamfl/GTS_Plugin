#pragma once
// Module that handles footsteps

#include "managers/impact.hpp"
#include "events.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	class FootStepManager : public EventListener {
		public:
			[[nodiscard]] static FootStepManager& GetSingleton() noexcept;

			virtual std::string DebugName() override;
			virtual void OnImpact(const Impact& impact) override;
	};
}
