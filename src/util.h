#pragma once
#include <SKSE/SKSE.h>

using namespace std;
using namespace RE;

namespace Gts {
  vector<ActorHandle> find_actors();
  float unit_to_meter(float unit);
  float meter_to_unit(float meter);
  void clone_bound(Actor* actor);
  BSBound* get_bound(Actor* actor);
  NiAVObject* get_bumper(Actor* actor);
}
