#pragma once
#include "events.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts
{

	class HitManager : public EventListener
	{
		public:
			[[nodiscard]] static HitManager& GetSingleton() noexcept;

			void HitEvent(const TESHitEvent* evt) override;
			void Update();
			private:
			bool CanGrow = false;
	};
}
