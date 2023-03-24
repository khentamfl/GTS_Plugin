#pragma once
#include "events.hpp"
#pragma once
#include "events.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	struct VoreData {
		VoreData(TESObjectREFR* tiny);
		TESObjectREFR* tiny;
	};

	class VoreHandler : public EventListener
	{
		public:
			[[nodiscard]] static VoreHandler& GetSingleton() noexcept;

			virtual std::string DebugName() override;
			virtual void Update() override;
            // Grab actor(s) for Vore
			static void EatActors(Actor* giant);

			static void GrabVoreActor(Actor* giant, Actor* tiny);

			static void ClearData(Actor* giant);

      		// Get Vore Actors (Since we can Vore more than one)
			static TESObjectREFR* GetHeldVoreObj(Actor* giant);
			static Actor* GetHeldVoreActors(Actor* giant);

			static void RegisterEvents();

			static void RegisterTriggers();

			std::unordered_map<Actor*, VoreData> data;
	};
}