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

	class HitManager : public EventListener, public BSTEventSink<InputEvent*>
	{
		public:
			[[nodiscard]] static HitManager& GetSingleton() noexcept;

			BSEventNotifyControl ProcessEvent(InputEvent* const* a_event, BSTEventSource<TESHitEvent*>* a_eventSource) override;
			void Start() override;
	};
}
