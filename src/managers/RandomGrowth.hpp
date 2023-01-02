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

			virtual std::string DebugName() override;
			virtual void Update() override;
			void CallShake(float value);
			void RandomizeGrowth(float value);

		private:
			bool AllowGrowth = false;
			bool CallInputGrowth = false;
			float growth_time = 0.0;
			float ShakePower = 0.0;
			float growth_time_input = 0.0;
			int Randomize = 0;
	};
}
