#pragma once
// Module that handles AttributeValues
#include "events.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {

	class RipClothManager : public EventListener {
		public:
			[[nodiscard]] static RipClothManager& GetSingleton() noexcept;

			void CheckRip();

		private:
            float clothtearthreshold = 2.5;

	};
}
