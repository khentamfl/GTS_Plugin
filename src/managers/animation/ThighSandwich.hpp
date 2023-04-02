#pragma once

#include "events.hpp"
#include "timer.hpp"
#include "spring.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	struct ThighData {
		ThighData(TESObjectREFR* tiny, bool RuneSpawn, bool RuneDespawn);
		Spring RuneController;
		TESObjectREFR* tiny;
		bool RuneSpawn;
		bool RuneDespawn;
	};

	class AnimationThighSandwich : public EventListener {
		public:
			[[nodiscard]] static AnimationThighSandwich& GetSingleton() noexcept;

			virtual std::string DebugName() override;
			virtual void Update() override;

			static void GrabActor(Actor* giant, TESObjectREFR* tiny);
			static void Release(Actor* giant);
			// Get object being held
			static TESObjectREFR* GetHeldObj(Actor* giant);
			static Actor* GetHeldActor(Actor* giant);

			static void RegisterEvents();

			static void RegisterTriggers();

			std::unordered_map<Actor*, ThighData> data;
	};
}
