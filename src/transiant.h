#pragma once
// Module that holds data that is not persistent across saves
#include <SKSE/SKSE.h>

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	struct TempActorData {
		float base_height;
	};

	class Transiant {
		public:
			[[nodiscard]] static Transiant& GetSingleton() noexcept;

			TempActorData* GetActorData(Actor* actor);
		private:

			mutable std::mutex _lock;
			std::unordered_map<RE::Actor*, TempActorData> _actor_data;
	};
}
