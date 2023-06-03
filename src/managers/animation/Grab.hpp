#pragma once
#include "events.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	struct GrabData {
		public:
			GrabData(TESObjectREFR* tiny, float strength);
			void SetGrabbed(bool decide);
			bool GetGrabbed();
			TESObjectREFR* tiny;
			float strength;

		private:
			bool grab;
	};

	class Grab : public EventListener
	{
		public:
			[[nodiscard]] static Grab& GetSingleton() noexcept;

			virtual std::string DebugName() override;

			static void RegisterEvents();
			static void RegisterTriggers();
			
			virtual void Update() override;
			virtual void Reset() override;
			virtual void ResetActor(Actor* actor) override;
			// Streangth is meant to be for a calculation of
			// escape chance currently unused
			static void GrabActor(Actor* giant, TESObjectREFR* tiny, float strength);
			static void GrabActor(Actor* giant, TESObjectREFR* tiny);
			static void Release(Actor* giant);

			bool GetHolding(Actor* giant);
			static void SetHolding(Actor* giant, bool decide);
			// Get object being held
			static TESObjectREFR* GetHeldObj(Actor* giant);
			// Same as `GetHeldObj` but with a conversion to actor if possible
			static Actor* GetHeldActor(Actor* giant);

			std::unordered_map<Actor*, GrabData> data;
	};
}
