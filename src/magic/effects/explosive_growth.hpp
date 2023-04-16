#pragma once
#include "magic/magic.hpp"
#include "timer.hpp"
// Module that handles Growth Spurt


using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	class ExplosiveGrowth : public Magic {
		public:
			virtual void OnUpdate() override;
			virtual void OnStart() override;
			virtual void OnFinish() override;


			virtual std::string GetName() override;

			ExplosiveGrowth(ActiveEffect* effect);

			virtual void DoGrowth(Actor* actor, float value);
			virtual void DoShrink(Actor* actor, float value);
		private:
			float power = 0.0;
			float grow_limit = 1.0;
			float growth_time = 0.0;
			float RequiredSizeChange = 0.0;
			float totalgainedsize = 0.0;
			Timer timer = Timer(2.33);
			Timer timerSound = Timer(0.33);
	};
}
