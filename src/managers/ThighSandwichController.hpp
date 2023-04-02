#pragma once

#include "events.hpp"
#include "timer.hpp"
#include "spring.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
    class ThighSandwichController : public EventListener  {
		public:
			[[nodiscard]] static ThighSandwichController& GetSingleton() noexcept;

			virtual std::string DebugName() override;

			static void GrabActor(Actor* giant, TESObjectREFR* tiny);
			static void Release(Actor* giant);
			
			std::vector<Actor*> GetSandwichTargetsInFront(Actor* pred, std::size_t numberOfPrey);
			static void StartSandwiching(Actor* pred, Actor* prey);
			static void CanSandwich(Actor* pred, Actor* prey);
	};
}
