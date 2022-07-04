#pragma once
// Module that finds nodes and node realated data
#include <SKSE/SKSE.h>

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
  void walk_nodes(Actor* actor);
  NiAVObject* find_node(Actor* actor, string& node_name);
  void query_nodes(Actor* actor);
  void clone_bound(Actor* actor);
  BSBound* get_bound(Actor* actor);
  NiAVObject* get_bumper(Actor* actor);
}
