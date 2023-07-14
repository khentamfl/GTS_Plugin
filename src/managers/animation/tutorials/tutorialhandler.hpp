#pragma once

#include "events.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace Gts {
    void TutorialMessage(std::string_view message, std::string_view type);
    void CheckTutorialTargets(Actor* actor);
}