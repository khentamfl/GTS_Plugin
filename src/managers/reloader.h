#pragma once
// Module that handles various reload events
#include <SKSE/SKSE.h>

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	class ReloadManager :
		public BSTEventSink<TESObjectLoadedEvent>,
		public BSTEventSink<TESCellFullyLoadedEvent>,
		public BSTEventSink<TESCellAttachDetachEvent>,
		public BSTEventSink<TESEquipEvent> {
		public:
			[[nodiscard]] static ReloadManager& GetSingleton() noexcept;
			void Initialize();

		protected:
			virtual BSEventNotifyControl ProcessEvent(const TESObjectLoadedEvent * evn, BSTEventSource<TESObjectLoadedEvent> * dispatcher) override;
			virtual BSEventNotifyControl ProcessEvent(const TESCellFullyLoadedEvent* evn, BSTEventSource<TESCellFullyLoadedEvent>* dispatcher) override;
			virtual BSEventNotifyControl ProcessEvent(const TESCellAttachDetachEvent* evn, BSTEventSource<TESCellAttachDetachEvent>* dispatcher) override;
			virtual BSEventNotifyControl ProcessEvent(const TESEquipEvent* evn, BSTEventSource<TESEquipEvent>* dispatcher) override;
	};
}
