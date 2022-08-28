#pragma once
// Module that handles debug drawing

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	class DDraw {
		public:
			[[nodiscard]] static DDraw& GetSingleton() noexcept;

			void Update();

			inline void Enable() {
				this->enabled.store(true);
			}
			inline void Disable() {
				this->enabled.store(false);
			}
		private:
			std::atomic_bool enabled = std::atomic_bool(true);
	};
}
