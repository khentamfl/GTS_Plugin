#pragma once

#include "events.hpp"
#include "timer.hpp"
#include "spring.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
    class AiData {
		public:
			AiData(Actor* giant);
			bool GetTimer(int type);
		private:
			ActorHandle giant;
			// Vore is done is sets with multiple actors if the giant is big
			// enough
            bool ActorsAreDead = false;
            bool IsThighSandwiching = false;
            inline static Timer ActionTimer = Timer(1.0);
	};
    class AiManager : public EventListener  {
		public:
			[[nodiscard]] static AiManager& GetSingleton() noexcept;

			virtual std::string DebugName() override;
            virtual void Update() override;
			virtual void Reset() override;
			virtual void ResetActor(Actor* actor) override;
			
			std::vector<Actor*> RandomStomp(Actor* pred, std::size_t numberOfPrey);
			bool CanStomp(Actor* pred, Actor* prey);

            AiData& GetAiData(Actor* giant);
            std::unordered_map<FormID, AiData> data_ai;
	};
}