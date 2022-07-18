#pragma once
// Module that handles various reload events
#include <SKSE/SKSE.h>

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	class ReloadManager :
		public BSTEventSink<TESObjectLoadedEvent>,
		public BSTEventSink<TESCellFullyLoadedEvent> {
		public:
			[[nodiscard]] static ReloadManager& GetSingleton() noexcept;

		protected:
			virtual BSEventNotifyControl ProcessEvent(const TESObjectLoadedEvent * evn, BSTEventSource<TESObjectLoadedEvent> * dispatcher) override;
			virtual BSEventNotifyControl ProcessEvent(const TESCellFullyLoadedEvent* evn, BSTEventSource<TESCellFullyLoadedEvent>* dispatcher) override;
	};
}
