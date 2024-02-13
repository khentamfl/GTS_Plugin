#pragma once

#include "events.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace Gts {

    enum class Difficulty_ByPC : std::string_view
    {
        "fDiffMultHPByPCVE" = 0,
        "fDiffMultHPByPCE" =  1,
        "fDiffMultHPByPCN" =  2,
        "fDiffMultHPByPCH" =  3,
        "fDiffMultHPByPCVH" = 4,
        "fDiffMultHPByPCL" =  5,
    };

    enum class Difficulty_ToPC : std::string_view
    {
        "fDiffMultHPToPCVE" = 0,
        "fDiffMultHPToPCE" =  1,
        "fDiffMultHPToPCN" =  2,
        "fDiffMultHPToPCH" =  3,
        "fDiffMultHPToPCVH" = 4,
        "fDiffMultHPToPCL" =  5,
    };

    enum class Difficulty : std::int32_t
	{
		Novice = 0,
		Apprentice = 1,
		Adept = 2,
		Expert = 3,
		Master = 4,
		Legendary = 5
	};

    float GetSettingValue(const char* setting);
    float GetDifficultyMultiplier(Actor* attacker, Actor* receiver);
}