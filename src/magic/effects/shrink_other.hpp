#pragma once
#include "magic/magic.hpp"
// Module that handles footsteps


using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	class ShrinkOther : public Magic {
		public:
			using Magic::Magic;

			virtual void OnUpdate() override;

			static bool StartEffect(EffectSetting* effect);
	};
}
