#pragma once
// Misc codes
#include <SKSE/SKSE.h>

using namespace std;
using namespace RE;
using namespace SKSE;

namespace Gts {
	inline static float* g_delta_time = (float*)REL::RelocationID(523660, 410199).address();

	inline std::string_view actor_name(Actor* actor) {
		if (actor) {
			auto ba = actor->GetActorBase();
			if (ba) {
				return ba->GetFullName();
			}
		}
		return "";
	}

	vector<Actor*> find_actors();
	float unit_to_meter(float unit);
	float meter_to_unit(float meter);
	void critically_damped(
		float& x,
		float& v,
		float x_goal,
		float halflife,
		float dt);
        
    inline bool logit(Actor* actor) {
        return (actor->formID == 0x14 || actor->IsTeammate())
    }
}
