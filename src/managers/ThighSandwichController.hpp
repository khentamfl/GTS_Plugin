#pragma once

#include "events.hpp"
#include "timer.hpp"
#include "spring.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
    class SandwichingData {
		public:
			SandwichingData(Actor* giant);
			// Adds a tiny to the list of actors
			// being eaten
			void AddTiny(Actor* tiny);
            void Remove(Actor* tiny);
            void EnableSuffocate(bool enable);
			void ManageScaleRune(bool enable);
			void ManageShrinkRune(bool enable);
			
			

			// Release all vories (shall fall into mouth with animation)
			void ReleaseAll();

			// Get a list of all actors currently being vored
			std::vector<Actor*> GetActors();

			// Update all things that are happening like
			// keeping them on the AnimObjectA and shrinking nodes
			void Update();
			void UpdateRune(Actor* giant);

		private:
			Actor* giant;
			// Sandwiching is done is sets with multiple actors if the giant is big
			// enough
			std::unordered_map<Actor*, Actor*> tinies = {};
            bool Suffocate = false;
			bool RuneScale = false;
			bool RuneShrink = false;

			Spring ScaleRune = Spring(0.0, 1.5);
			Spring ShrinkRune = Spring(1.0, 1.5);

			// True if in grabbed state
			bool allGrabbed = false;
	};
    class ThighSandwichController : public EventListener  {
		public:
			[[nodiscard]] static ThighSandwichController& GetSingleton() noexcept;

			virtual std::string DebugName() override;
            virtual void Update() override;
			
			std::vector<Actor*> GetSandwichTargetsInFront(Actor* pred, std::size_t numberOfPrey);
			static void StartSandwiching(Actor* pred, Actor* prey);
			bool CanSandwich(Actor* pred, Actor* prey);

            SandwichingData& GetSandwichingData(Actor* giant);

            std::unordered_map<Actor*, SandwichingData> data;
	};
}
