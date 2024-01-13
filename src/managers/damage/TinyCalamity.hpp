

#pragma once

#include "events.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace Gts {
    void TinyCalamity_ExplodeActor(Actor* giant, Actor* tiny);
    void TinyCalamity_SeekActors(Actor* giant);
    void TinyCalamity_CrushCheck(Actor* giant, Actor* tiny);
    void TinyCalamity_BonusSpeed(Actor* giant);
}