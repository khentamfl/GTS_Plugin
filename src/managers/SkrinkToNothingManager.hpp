#pragma once
// Module that handles footsteps
#include "events.hpp"
#include "timer.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {

	enum class ShrinkState {
		Healthy,
		Shinking,
		Shrinked
	};

	class ShrinkData {
		public:
			ShrinkData(Actor* giant, Actor* tiny);

			CrushState state;
			Timer delay;
			Actor* giant;
	};

	class ShrinkToNothingManager : public EventListener {
		public:
			[[nodiscard]] static ShrinkToNothingManager& GetSingleton() noexcept;

			virtual std::string DebugName() override;
			virtual void Update() override;
			virtual void Reset() override;
			virtual void ResetActor(Actor* actor) override;

			static bool CanShrink(Actor* giant, Actor* tiny);
			static bool AlreadyShrinked(Actor* actor);
			static void Shrink(Actor* giant, Actor* tiny);
		private:
			std::unordered_map<Actor*, ShrinkData> data;
	};
}
