#pragma once
// Module that holds data that is loaded at runtime
// This includes various forms
#include <SKSE/SKSE.h>

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	class Runtime {
		public:
			[[nodiscard]] static Runtime& GetSingleton() noexcept;

			void Load();

			BGSImpactDataSet* FootstepL = nullptr;
			BGSImpactDataSet* FootstepR = nullptr;
	};
}
