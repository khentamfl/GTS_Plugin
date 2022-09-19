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

		virtual Result ProcessEvent(RE::InputEvent* const* a_event, RE::BSTEventSource<RE::InputEvent*>* a_eventSource) override;

        void Start() override {
            Register();
        };
        
		static void Register()
		{
			auto deviceManager = RE::BSInputDeviceManager::GetSingleton();
			deviceManager->AddEventSink(InputManager::GetSingleton());
		}
	};
}