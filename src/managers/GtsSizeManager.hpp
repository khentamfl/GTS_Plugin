#pragma once
// Module that handles AttributeAdjustment


using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {

	class SizeManager {
		public:
			[[nodiscard]] static SizeManager& GetSingleton() noexcept;

			void UpdateSize(Actor* Target);
	};
}
