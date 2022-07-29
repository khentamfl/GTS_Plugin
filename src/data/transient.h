#pragma once
// Module that holds data that is not persistent across saves
#include <SKSE/SKSE.h>

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	struct TempActorData {
		float base_height;
		float base_volume;
		float last_hh_adjustment;
		float total_hh_adjustment;
		float base_walkspeedmult;
	};

	class Transient {
		public:
			[[nodiscard]] static Transient& GetSingleton() noexcept;

			TempActorData* GetActorData(Actor* actor);

			void Clear();
		private:

			mutable std::mutex _lock;
			std::unordered_map<FormID, TempActorData> _actor_data;
	};
}
