#pragma once

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts
{
	class InputManager : public RE::ControlMap
	{
	public:

		inline float GetKeyPress() {
            auto PressedKey = RE::ControlMap::Events->inputKey;
            if (!PressedKey)    {
                return 0;
            }
            else {
            return PressedKey;
            }
        }

        inline std::string GetActionString() {
            auto Action = RE::ControlMap::Events->eventID;
            if (!Action)    {
                return "None";
            }
            else {
            return Action;
            }
        }
    }
}