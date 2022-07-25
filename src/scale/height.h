#pragma once
// Handles the various methods of scaling an actor
#include <SKSE/SKSE.h>

using namespace std;
using namespace RE;
using namespace SKSE;

namespace Gts {
	void set_target_height(Actor* actor, float height);
	float get_target_height(Actor* actor);
	void mod_target_height(Actor* actor, float amt);

	void set_max_height(Actor* actor, float height);
	float get_max_height(Actor* actor);
	void mod_max_height(Actor* actor, float amt);

	float get_visual_height(Actor* actor);
}
