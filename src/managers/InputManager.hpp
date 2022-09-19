#pragma once
#include "data/transient.hpp"
#include "events.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts
{
	using EventResult = RE::BSEventNotifyControl;

	class InputManager : public EventListener, public BSTEventSink<InputEvent*>
	{
		public:
			[[nodiscard]] static InputManager& GetSingleton() noexcept;

			EventResult ProcessEvent(RE::InputEvent* const* a_event, RE::BSTEventSource<RE::InputEvent*> a_source) override;
			void Start() override;
	};
}
