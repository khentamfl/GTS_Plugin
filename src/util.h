#pragma once
// Misc codes
#include <SKSE/SKSE.h>

using namespace std;
using namespace RE;
using namespace SKSE;

namespace Gts {
  vector<ActorHandle> find_actors();
  float unit_to_meter(float unit);
  float meter_to_unit(float meter);
}
