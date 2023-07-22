#pragma once

#include "events.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace Gts {

    void DoCrawlingSounds(Actor* actor, float scale, NiAVObject* node, FootEvent foot_kind);
    void DoCrawlingFunctions(Actor* actor, float scale, float multiplier, CrawlEvent kind, std::string_view tag, float launch_dist, float damage_dist);
    void DoCrawlingDamage(Actor* giant, float radius, float damage, NiAVObject* node, float random, float bbmult);
    void ApplyAllCrawlingDamage(Actor* giant, float damage, int random, float bonedamage)
}