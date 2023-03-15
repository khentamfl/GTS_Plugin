#pragma once
#include "events.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

// namespace Gts {
// 	struct GrabData {
// 		GrabData(Actor* tiny);
// 		Actor* tiny;
// 	};
//
// 	class Grab : public EventListener
// 	{
// 		public:
// 			[[nodiscard]] static Grab& GetSingleton() noexcept;
//
// 			virtual std::string DebugName() override;
// 			virtual void Update() override;
// 			void GrabActor(Actor* tiny);
// 			void CrushActors();
// 			void Clear();
// 		private:
// 			std::unordered_map<Actor*, GrabData> data;
// 	};
// }
