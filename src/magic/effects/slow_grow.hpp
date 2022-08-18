#pragma once
#include "magic/magic.hpp"
// Module that handles footsteps


using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	class SlowGrow : public Magic {
		public:
			using Magic::Magic;

			virtual void OnUpdate() override;

			virtual std::string GetName() override;

			static bool StartEffect(EffectSetting* effect);

		private:
            bool WasDualCasted = false;	
	};
}
