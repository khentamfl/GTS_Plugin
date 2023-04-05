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
            UpdateEmotions(Actor* giant);
			EmotionData(Actor* giant);
			void Update();
			Actor* giant;
			bool AllowEmotionEdit = false;
            std::vector<Spring> Phenomes = {
                Spring(0.0, 0.0), //0 - 0.0
                Spring(0.0, 0.0), //0 - 0.5
                Spring(0.0, 0.0), //0 - 1.0
                Spring(0.0, 0.0), //0 - 0.8
            };
            std::vector<Spring> Modifiers = {
                Spring(0.0, 0.0), // 0 - 0.8
                Spring(0.0, 0.0), // 0 - 0.8
            };
	};
	class EmotionManager : public EventListener {
		public:
			[[nodiscard]] static EmotionManager& GetSingleton() noexcept;

			virtual std::string DebugName() override;
			virtual void Update() override;

			EmotionData& GetGiant(Actor* giant);
            EmotionData& Clear(Actor* giant);

		private:
			std::map<Actor*, EmotionData> data;
	};
}
