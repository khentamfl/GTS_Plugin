#pragma once
// Module that handles AttributeAdjustment
#include "events.hpp"
#include "spring.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	class EmotionData {
		public:
			void OverridePhenome(int number, float power, float hl, float tg);
			void OverrideModifier(int number, float power, float hl, float tg);
	
	};
	class EmotionManager : public EventListener {
		public:
			[[nodiscard]] static EmotionManager& GetSingleton() noexcept;

			virtual std::string DebugName() override;
	};
}
