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
		Shrinking,
		Shrinked
	};

	class ShrinkData {
		public:
			ShrinkData(ActorHandle* giant, ActorHandle* tiny);

			ShrinkState state;
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

			static bool CanShrink(ActorHandle* giant, ActorHandle* tiny);
			static bool AlreadyShrinked(ActorHandle* actor);
			static void Shrink(ActorHandle* giant, ActorHandle* tiny);
			static void AdjustGiantessSkill(ActorHandle* Caster, ActorHandle* Target);
		private:
			std::unordered_map<Actor*, ShrinkData> data;
	};
}
