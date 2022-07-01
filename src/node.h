#pragma once
#include <SKSE/SKSE.h>

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
  void walk_nodes(Actor* actor);
  NiAVObject* find_node(Actor* actor, string& node_name);
}
