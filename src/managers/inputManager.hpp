#pragma once

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts
{
	class InputManager
	{
	public:

        inline std::string GetActionString() {
            auto Action = RE::UserEvents::GetSingleton()->activate;
            if (!Action)    {
                return "None";
            }
            else {
            return std::Action;
            }
        }
    }
}