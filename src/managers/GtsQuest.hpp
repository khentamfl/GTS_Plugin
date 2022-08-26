#pragma once
// Module that handles footsteps


using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	class QuestManager {
		public:
			[[nodiscard]] static QuestManager& GetSingleton() noexcept;

			void Update();
	}
}
