#pragma once
#include "events.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	/*struct GrabData {
		GrabData(TESObjectREFR* tiny, float strength);
		TESObjectREFR* tiny;
		float strength;
	};*/

	class ActorVore : public EventListener
	{
		public:
			[[nodiscard]] static ActorVore& GetSingleton() noexcept;

			virtual std::string DebugName() override;
			virtual void Update() override;
            // Grab actor(s) for Vore
			static void GrabVoreActor(Actor* giant, Actor* tiny);

      		// Get Vore Actors (Since we can Vore more than one)
			static Actor* GetHeldVoreActors(Actor* giant);

			//std::unordered_map<Actor*, GrabData> data; <-- Should be same principle as Grab.hpp/cpp i think
	};
}