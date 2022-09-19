#pragma once

#include "events.hpp"

using namespace std;
using namespace RE;

namespace Gts
{
	using Result = RE::BSEventNotifyControl;

	class InputEventHandler: public EventListener, public BSTEventSink<InputEvent*>
	{
	public:
        [[nodiscard]] static InputManager& GetSingleton() noexcept;

		virtual Result::ProcessEvent(RE::InputEvent* const* a_event, RE::BSTEventSource<RE::InputEvent*>*) {
  		if (!a_event) {
   		 return EventResult::kContinue;
 		 }
  		for (auto event = *a_event; event; event = event->next) {
   		 if (event->eventType != EventType::kButton) {
   		 continue;
    	}
    		auto buttonEvent = a_event->AsButtonEvent();
    // You now have the button event
 		 }
	}
        void Start() override {
            Register();
        };
        
		std::string GetActionString();
		void DetectInput();

		static void Register()
		{
			auto deviceManager = RE::BSInputDeviceManager::GetSingleton();
			deviceManager->AddEventSink(InputManager::GetSingleton());
		}
	};
}