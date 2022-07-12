#pragma once
// Module that finds nodes and node realated data
#include <SKSE/SKSE.h>

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	void walk_nodes(Actor* actor);
	NiAVObject* find_node(Actor* actor, std::string_view node_name, bool first_person = false);
	void scale_hkpnodes(Actor* actor, float prev_scale, float new_scale);
	void clone_bound(Actor* actor);
	BSBound* get_bound(Actor* actor);
	NiAVObject* get_bumper(Actor* actor);
}
