#include "modevent.h"

using namespace SKSE;
using namespace RE;

namespace Gts {
	ModEventManager& ModEventManager::GetSingleton() noexcept {
		static ModEventManager instance;
		return instance;
	}

	ModEventManager::ModEventManager() : m_onfootstep("OnFootstep"sv)
	{
	}
}
