#include "util.h"

using namespace Gts;

namespace Gts {
  /**
	 * Find actors in ai manager that are loaded
	 */
	vector<ActorHandle> find_actors() {
		vector<ActorHandle> result;

		auto process_list = ProcessLists::GetSingleton();
		for (ActorHandle actor_handle: process_list->highActorHandles)
		{
			auto actor = actor_handle.get();
			if (actor && actor->Is3DLoaded())
			{
				result.push_back(actor_handle);
			}
		}

		return result;
	}

	float unit_to_meter(float unit) {
		// Game reports that the height of a slaughterfish is 0.31861934
		// From inspecting the bounding box of the slaughterfish and applying
		// base actor scales the unit height is 22.300568
		// Assuming 0.31861934 is meters and that bouding box is in model unit space
		// then the conversion factor is 70
		// Slaughterfish was chosen because it has scales of 1.0 (and was in my worldspace)
		// The scaling factor of 70 also applies to actor heights (once you remove)
		// race specific height scaling
		return unit / 70.0;
	}

	float meter_to_unit(float meter) {
		// Game reports that the height of a slaughterfish is 0.31861934
		// From inspecting the bounding box of the slaughterfish and applying
		// base actor scales the unit height is 22.300568
		// Assuming 0.31861934 is meters and that bouding box is in model unit space
		// then the conversion factor is 70
		// Slaughterfish was chosen because it has scales of 1.0 (and was in my worldspace)
		// The scaling factor of 70 also applies to actor heights (once you remove)
		// race specific height scaling
		return meter * 70.0;
	}
}
