#pragma once
#include "magic/magic.hpp"
// Module that handles footsteps


using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	class Absorb : public Magic {
		public:
			virtual void OnUpdate() override;

			static bool StartEffect(EffectSetting* effect);
		private:
			bool true_absorb = false;
	};
}
