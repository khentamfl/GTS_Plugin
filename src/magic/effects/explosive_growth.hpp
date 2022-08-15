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

			static bool StartEffect(EffectSetting* effect);

			ExplosiveGrowth(ActiveEffect* effect);
		private:
			float power = 0.0;
	};
}
