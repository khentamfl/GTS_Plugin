#pragma once
// Module that finds nodes and node realated data
using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	enum Person {
		First,
		Third,
		Any,
		Current,
	};
	void walk_nodes(Actor* actor);
	NiAVObject* find_node(Actor* actor, std::string_view node_name, Person person);
	NiAVObject* find_node_regex(Actor* actor, std::string_view node_regex, Person person);
	void clone_bound(Actor* actor);
	BSBound* get_bound(Actor* actor);
	NiAVObject* get_bumper(Actor* actor);

	void update_node(NiAVObject* node);
}
