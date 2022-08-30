#pragma once
// Module that handles AttributeValues


using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {

	class RandomGrowth {
		public:
			[[nodiscard]] static RandomGrowth& GetSingleton() noexcept;

			void Update();

		private:
			bool AllowGrowth = false;
			float growth_time = 0.0;
	};
}
