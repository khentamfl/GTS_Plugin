#include "managers/inputManager.hpp"

namespace Gts
{
	std::uint32_t InputEventHandler::GetKeyPress(RE::INPUT_DEVICE a_device)
	{
		using DeviceType = RE::INPUT_DEVICE;

        case DeviceType::kKeyboard:
			return kKeyboardOffset;
		break;

		return 0;
	    }
}
