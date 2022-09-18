#pragma once

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts
{
	class InputManager
	{
	public:
		inline float GetKeyPress() {
            auto Keys = RE::ControlMap::GetSingleton();
            auto PressedKey = Keys->inputKey;
            if (!PressedKey)    {
                return 0;
            }
            else {
            return PressedKey;
            }
        }

        inline std::string GetActionString() {
            auto Keys = RE::ControlMap::GetSingleton();
            auto Action = Keys->eventID;
            if (!Action)    {
                return "None";
            }
            else {
            return std::Action;
            }
        }
    }
}