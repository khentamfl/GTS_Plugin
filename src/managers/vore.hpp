#pragma once
#include "events.hpp"
#include "spring.hpp"

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
			// Swallow and start the digestion (buffs)
			void Swallow();
			// Finishes the process
			// kill/shrinks all actors and gains buffs
			void KillAll();
			// Protects from being vored. Used to disallow same-target vore by 2 actors
			void ProtectFromVore();

			// Grab all vories
			void GrabAll();

			// Release all vories (shall fall into mouth with animation)
			void ReleaseAll();

			bool GetTimer();

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

			inline static Timer moantimer = Timer(6.0);

			// True if in grabbed state
			bool allGrabbed = false;
	};

	enum VoreBuffState {
    	Starting,
    	RampUp,
		Running,
    	RampDown,
		Finishing,
		Done,
	};
	struct VoreBuff {
		VoreBuffState state = VoreBuffState::Starting;
		Actor* giant = nullptr;
		Actor* tiny = nullptr;
    	float duration;
		float restorePower = 0.0; // Amount of health to restore TOTAL
		float sizePower = 0.0; // Amount of size to gain TOTAL
		float tinySize = 0.0;
		std::string_view tiny_name;
		

		// Used to track how much time has passed (abusing Spring code)
		// The half life will tell the half the duration
		Spring factor = Spring(0.0, 15.0);
		float appliedFactor = 0.0;

    // Create the buff
    //
    // This also calculates the restorePower etc and the duration
		VoreBuff(Actor* giant, Actor* tiny);

    // Called every frame until we are done
		void Update();

    // Reports true when done so that it can be removed
		bool Done();
	};

	class Vore : public EventListener
	{
		public:
			[[nodiscard]] static Vore& GetSingleton() noexcept;

			virtual std::string DebugName() override;
			virtual void Reset() override;
			virtual void ResetActor(Actor* actor) override;
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

			void AddVoreBuff(Actor* giant, Actor* tiny);

		private:
			std::unordered_map<Actor*, VoreData> data;
			std::unordered_map<Actor*, VoreBuff> buffs;
	};
}
