#pragma once
#include "magic/magic.h"
// Module that handles footsteps
#include <SKSE/SKSE.h>

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	class ShrinkButton : Magic {
		public:
			virtual void OnUpdate() override;

			static bool StartEffect(EffectSetting* effect);
	};
}
