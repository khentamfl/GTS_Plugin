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
			inline static float BonusPower = 1.0;
			inline static float GrowthTick = 0.0;
	};
}
