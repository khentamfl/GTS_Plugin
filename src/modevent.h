#pragma once
// Module that handles footsteps
#include <SKSE/SKSE.h>

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	class ModEventManager {
		public:
			[[nodiscard]] static ModEventManager& GetSingleton() noexcept;

			RegistrationSet<Actor*, std::string> m_onfootstep;
		private:
			ModEventManager();
	};
}
