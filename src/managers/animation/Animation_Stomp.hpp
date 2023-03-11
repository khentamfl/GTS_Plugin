#pragma once
#include "events.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts
{
	class Stomp : public EventListener
	{
		public:
			[[nodiscard]] static Stomp& GetSingleton() noexcept;

			virtual std::string DebugName() override;
			void ActorAnimEvent(Actor* actor, const std::string_view& tag, const std::string_view& payload) override;
			void ApplyStomp(Actor* actor, std::string_view condition);
	};
}
