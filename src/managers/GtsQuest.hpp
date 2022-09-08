#pragma once
// Module that handles footsteps
#include "events.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	class QuestManager : public EventListener {
		public:
			[[nodiscard]] static QuestManager& GetSingleton() noexcept;

			virtual void Update() override;
	};
}
