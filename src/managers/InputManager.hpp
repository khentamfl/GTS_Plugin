#pragma once
#include "data/transient.hpp"
#include "events.hpp"
#include "timer.hpp"

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

			BSEventNotifyControl ProcessEvent(InputEvent* const* a_event, BSTEventSource<InputEvent*>* a_eventSource) override;
			void Start() override;
		private:	
			Timer timer = Timer(1.2);
			Timer voretimer = Timer(0.2);
			Timer timergrowth = Timer(2.00);
			float TickCheck = 0.0;
			float growth_time = 0.0;
			bool BlockShake = false;
	};
}
