#pragma once
#include "events.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts
{
	class AnimationInputManager : public EventListener
	{
		public:
			[[nodiscard]] static AnimationInputManager& GetSingleton() noexcept;

			virtual std::string DebugName() override;
	};
}
