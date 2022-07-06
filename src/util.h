#pragma once
// Misc codes
#include <SKSE/SKSE.h>

using namespace std;
using namespace RE;
using namespace SKSE;

namespace Gts {
	inline static float* g_delta_time = (float*)REL::RelocationID(523660, 410199).address();

	vector<ActorHandle> find_actors();
	float unit_to_meter(float unit);
	float meter_to_unit(float meter);
	void critically_damped(
		float& x,
		float& v,
		float x_goal,
		float halflife,
		float dt);
}
