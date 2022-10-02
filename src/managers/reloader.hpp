#pragma once
// Module that handles various reload events


using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	class ReloadManager :
		public BSTEventSink<TESHitEvent>,
		public BSTEventSink<TESObjectLoadedEvent>,
		public BSTEventSink<TESResetEvent>,
		public BSTEventSink<TESEquipEvent> {
		public:
			[[nodiscard]] static ReloadManager& GetSingleton() noexcept;
			void Initialize();

		protected:
			virtual BSEventNotifyControl ProcessEvent(const TESHitEvent * evn, BSTEventSource<TESHitEvent> * dispatcher) override;
			virtual BSEventNotifyControl ProcessEvent(const TESObjectLoadedEvent * evn, BSTEventSource<TESObjectLoadedEvent> * dispatcher) override;
			virtual BSEventNotifyControl ProcessEvent(const TESResetEvent* evn, BSTEventSource<TESResetEvent>* dispatcher) override;
			virtual BSEventNotifyControl ProcessEvent(const TESEquipEvent* evn, BSTEventSource<TESEquipEvent>* dispatcher) override;
	};
}
