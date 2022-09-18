#pragma once


namespace Gts
{
	using Events = RE::UserEventEnabled;

	class InputManager : public RE::BSTEventSink<RE::InputEvent*>
	{
	public:

		inline float GetKeyPress() {
            auto PressedKey = Events->inputKey;
            if (!PressedKey)    {
                return 0;
            }
            else
            return PressedKey;
        }

        inline std::string GetActionString() {
            auto Action = Events->eventID;
            if (!Action)    {
                return "None";
            }
            else
            return Action;
        }
    }
}