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
            auto Result = std::string Action;
            return Result;
        }
    }
}