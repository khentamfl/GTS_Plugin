#pragma once
// Module that handles AttributeValues


using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	class AttributeManager {
		public:
			[[nodiscard]] static AttributeManager& GetSingleton() noexcept;

			void Update();
	};
}