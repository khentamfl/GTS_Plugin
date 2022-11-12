#include "managers/GtsQuest.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "magic/effects/common.hpp"

using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace Gts {
	QuestManager& QuestManager::GetSingleton() noexcept {
		static QuestManager instance;
		return instance;
	}

	std::string QuestManager::DebugName() {
		return "QuestManager";
	}

	void QuestManager::Update() {
	}
}
