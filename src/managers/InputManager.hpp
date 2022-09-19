#pragma once
#include "data/transient.hpp"
#include "events.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts
{
	using Result = RE::BSEventNotifyControl;

	class InputManager : public EventListener, public BSTEventSink<InputEvent*>
	{
		public:
			[[nodiscard]] static InputManager& GetSingleton() noexcept;

			EventResult ProcessEvent(RE::InputEvent* const* a_event, RE::BSTEventSource<RE::InputEvent*>) override;
			void Start() override;
	};
}
