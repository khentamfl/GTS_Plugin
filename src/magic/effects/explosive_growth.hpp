#pragma once
#include "magic/magic.hpp"
// Module that handles footsteps


using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	class ExplosiveGrowth : public Magic {
		public:
			virtual void OnUpdate() override;
			virtual void OnStart() override;

			virtual std::string GetName() override;

			static bool StartEffect(EffectSetting* effect);

			ExplosiveGrowth(ActiveEffect* effect);
		private:
			float power = 0.0;
			float grow_limit = 1.0;
			float growth_time = 0.0;
	};
}
