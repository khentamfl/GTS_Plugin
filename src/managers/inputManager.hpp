#pragma once


using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts
{
	class InputManager
	{
	public:

        [[nodiscard]] static InputManager& GetSingleton() noexcept;
    
        inline string GetActionString() {
            
            auto Action = RE::UserEvents::GetSingleton()->activate;
            auto Result = std::basic_string Action;
            return Result.c_str();
        }
    }
}