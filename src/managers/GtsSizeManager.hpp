#pragma once
// Module that handles AttributeValues


using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {

	class SizeManager {
		public:
			[[nodiscard]] static SizeManager& GetSingleton() noexcept;

			void Update(Actor* Target);

		private:
			bool AllowGrowth = false;
			float growth_time = 0.0;
	};
}
