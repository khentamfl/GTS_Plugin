#pragma once

#include "events.hpp"
#include <SKSE/Version.h>


using namespace std;
using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace Gts {
    using UEFlag = UserEvents::USER_EVENT_FLAG;
    void ToggleControls(UEFlag a_flag, bool a_enable);
    void ManageControls();
}