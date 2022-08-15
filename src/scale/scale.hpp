#pragma once
// Handles the various methods of scaling an actor
#include <SKSE/SKSE.h>

using namespace std;
using namespace RE;
using namespace SKSE;

namespace Gts {
	void set_target_scale(Actor* actor, float height);
	float get_target_scale(Actor* actor);
	void mod_target_scale(Actor* actor, float amt);

	void set_max_scale(Actor* actor, float height);
	float get_max_scale(Actor* actor);
	void mod_max_scale(Actor* actor, float amt);

	float get_visual_scale(Actor* actor);
	float get_effective_scale(Actor* actor);
}
