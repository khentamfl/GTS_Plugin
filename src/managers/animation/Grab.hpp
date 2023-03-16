#pragma once
#include "events.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	struct GrabData {
		GrabData(TESObjectREFR* tiny, float strength);
		TESObjectREFR* tiny;
		float strength;
	};

	class Grab : public EventListener
	{
		public:
			[[nodiscard]] static Grab& GetSingleton() noexcept;

			virtual std::string DebugName() override;
			virtual void Update() override;
			// Streangth is meant to be for a calculation of
			// escape chance currently unused
			static void GrabActor(Actor* giant, TESObjectREFR* tiny, float strength);
			static void GrabActor(Actor* giant, TESObjectREFR* tiny);
			static void Release(Actor* giant);
      static Actor* GetHeldActor(Actor* giant);

      std::unordered_map<Actor*, GrabData> data;
	};
}
