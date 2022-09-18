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
    
        std::string GetActionString() {
            
            auto Action = RE::UserEvents::GetSingleton()->activate;
            auto Result = Action.c_str();
            return Result;
        }

        //float GetPressedButton() {
          //  auto KeyPress = RE::ControlMap::GetSingleton()->inputKey;
          //  auto TheKey = KeyPress.c_str();
           // return TheKey;
        //}
   // };
}