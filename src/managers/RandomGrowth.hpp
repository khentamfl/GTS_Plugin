#pragma once
// Module that handles AttributeValues
#include "events.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {

	class RandomGrowth : public EventListener {
		public:
			[[nodiscard]] static RandomGrowth& GetSingleton() noexcept;

			virtual void Update() override;

		private:
			bool AllowGrowth = false;
			float growth_time = 0.0;
	};
}
