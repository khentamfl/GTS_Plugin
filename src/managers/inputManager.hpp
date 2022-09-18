#pragma once

namespace Gts
{
	using EventResult = RE::BSEventNotifyControl;

	class InputManager : public RE::BSTEventSink<RE::InputEvent*>
	{
	public:
		virtual EventResult CheckButton(RE::InputEvent* const* a_event, RE::BSTEventSource<RE::InputEvent*>* a_eventSource) override;

		static void Register()
		{
			auto deviceManager = RE::BSInputDeviceManager::GetSingleton();
			deviceManager->AddEventSink(InputEventHandler::GetSingleton());
		}

		static std::uint32_t GetKeyPress(RE::INPUT_DEVICE a_device);

}