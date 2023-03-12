#pragma once
#include "events.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts
{

    class GrabData {
		public:
			GrabData(Actor* tiny);
			Actor* tiny;
	    };
	class Grab : public EventListener
	{
		public:
			[[nodiscard]] static Grab& GetSingleton() noexcept;

			virtual std::string DebugName() override;
			virtual void Update() override;
			void GrabActor(Actor* tiny, std::string_view findbone);
            void CrushActors();
        private:
			std::unordered_map<Actor*, GrabData> data; 
	};
}
