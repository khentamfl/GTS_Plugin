#pragma once

#include "events.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace Gts {
    static std::string_view GetDeathNodeName(DamageSource cause);
    static NiAVObject* GetDeathNode(DamageSource cause);
    void ReportDeath(Actor* giant, Actor* tiny, DamageSource cause);
}