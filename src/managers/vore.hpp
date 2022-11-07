#pragma once
#include "events.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts
{

	class Vore : public EventListener
	{
		public:
			[[nodiscard]] static Vore& GetSingleton() noexcept;

			// Get's vore target for player using the crosshair
			// This will only return actors with appropiate distance/scale
			// as based on `CanVore`
			Actor* GetPlayerVoreTarget();

			// Get's vore target for any actor based on direction they are facing
			// This will only return actors with appropiate distance/scale
			// as based on `CanVore`
			Actor* GetVoreTarget(Actor* pred);

			// Check if they can vore based on size difference and reach distance
			bool CanVore(Actor* pred, Actor* prey);

			// Do the vore (this has no checks make sure they can vore with CanVore first)
			void StartVore(Actor* pred, Actor* prey);
	};
}
