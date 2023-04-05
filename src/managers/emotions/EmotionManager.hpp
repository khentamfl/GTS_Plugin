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
			EmotionData(Actor* giant);
            void UpdateEmotions(Actor* giant);
			void OverridePhenome(int number, float power, float hl, float tg);
			void OverrideModifier(int number, float power, float hl, float tg);
			
			void Update();
			Actor* giant;
			bool AllowEmotionEdit = false;

			private:
				Spring Phenome0 = Spring(0.0, 0.16);
				Spring Phenome1 = Spring(0.0, 0.16);
				Spring Phenome5 = Spring(0.0, 0.16);

				Spring Modifier0 = Spring(0.0, 0.25);
				Spring Modifier1 = Spring(0.0, 0.25);
	};
	class EmotionManager : public EventListener {
		public:
			[[nodiscard]] static EmotionManager& GetSingleton() noexcept;

			virtual std::string DebugName() override;
			virtual void Update() override;

			EmotionData& GetGiant(Actor* giant);

		private:
			std::map<Actor*, EmotionData> data;
	};
}
