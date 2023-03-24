#pragma once
#include "events.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts
{
  // Represents current vore data for an actor
  class VoreData {
    public:
      VoreData(Actor* giant);
      // Adds a tiny to the list of actors
      // being eaten
      void AddTiny(Actor* tiny);
      // Enables/diables the shrink zone
      void EnableMouthShrinkZone(bool enabled);
      // Finishes the process
      // kill/shrinks all actors and gains buffs
      void KillAll();

      // Get a list of all actors currently being vored
      std::vector<Actor*> GetVories();

      // Update all things that are happening like
      // keeping them on the AnimObjectA and shrinking nodes
      void Update();

    private:
      Actor* giant;
      // Vore is done is sets with multiple actors if the giant is big
      // enough
      std::unordered_map<Actor*, Actor*> tinies = {};

      // If true the mouth kill zone is on and we shrink nodes entering the mouth
      bool killZoneEnabled = false;
  };

	class Vore : public EventListener
	{
		public:
			[[nodiscard]] static Vore& GetSingleton() noexcept;

			virtual std::string DebugName() override;
      virtual void DataReady() override;
			virtual void Update() override;

			void RandomVoreAttempt(Actor* pred);

			// Get's vore target for pred using the crosshair
			// This will only return actors with appropiate distance/scale
			// as based on `CanVore`
			Actor* GeVoreTargetCrossHair(Actor* pred);
			// The varient get's multiple targets
			std::vector<Actor*> GeVoreTargetsCrossHair(Actor* pred, std::size_t numberOfPrey);

			// Get's vore target for any actor based on direction they are facing
			// This will only return actors with appropiate distance/scale
			// as based on `CanVore`
			Actor* GetVoreTargetInFront(Actor* pred);

			// Get's vore target for any actor based on distance from pred
			// This will only return actors with appropiate distance/scale
			// as based on `CanVore`  and can return multiple targets
			std::vector<Actor*> GetVoreTargetsInFront(Actor* pred, std::size_t numberOfPrey);

			// Get's vore target for any actor based on direction they are facing
			// This will only return actors with appropiate distance/scale
			Actor* GetVoreTargetAround(Actor* pred);

			// Get's vore target for any actor based on distance from pred
			// This will only return actors with appropiate distance/scale
			// as based on `CanVore` and can return multiple targets
			std::vector<Actor*> GetVoreTargetsAround(Actor* pred, std::size_t numberOfPrey);

			// Check if they can vore based on size difference and reach distance
			bool CanVore(Actor* pred, Actor* prey);

			// Do the vore (this has no checks make sure they can vore with CanVore first)
			void StartVore(Actor* pred, Actor* prey);

      // Gets the current vore data of a giant
      VoreData& GetVoreData(Actor* giant);

    private:
      std::unordered_map<Actor*, VoreData> data;
	};
}
