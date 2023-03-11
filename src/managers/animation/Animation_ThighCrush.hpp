#pragma once
#include "events.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts
{
	class ThighCrush : public EventListener
	{
		public:
			[[nodiscard]] static ThighCrush& GetSingleton() noexcept;

			virtual std::string DebugName() override;
            void ActorAnimEvent(Actor* actor, const std::string_view& tag, const std::string_view& payload);
			void ApplyThighCrush(Actor* actor, std::string_view condition);
			void AdjustAnimSpeed(Actor* actor, float bonus);
	};
}
