#pragma once
// Module that finds nodes and node realated data
#include <SKSE/SKSE.h>

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	void walk_nodes(Actor* actor);
	NiAVObject* find_node(Actor* actor, std::string_view node_name, bool first_person = false);
	NiAVObject* find_node_regex(Actor* actor, std::string_view node_regex, bool first_person = false);
	NiAVObject* find_node_any(Actor* actor, std::string_view node_name);
	NiAVObject* find_node_regex_any(Actor* actor, std::string_view node_regex);
	void scale_hkpnodes(Actor* actor, float prev_scale, float new_scale);
	void clone_bound(Actor* actor);
	BSBound* get_bound(Actor* actor);
	NiAVObject* get_bumper(Actor* actor);

	void update_node(NiAVObject* node);
}
