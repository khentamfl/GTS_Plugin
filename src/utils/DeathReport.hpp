#pragma once

#include "events.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace Gts {
    void ReportDeath(Actor* giant, Actor* tiny, DamageSource cause);
}