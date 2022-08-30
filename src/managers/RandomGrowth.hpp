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
			enum State {
				Idle,
				Working,
			};
			State state = State::Idle;
			float growth_time = 0.0;
	};
}
