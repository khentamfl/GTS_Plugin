#pragma once
#include "magic/magic.hpp"
// Module that handles footsteps
#include <SKSE/SKSE.h>

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	class ShrinkOther : Magic {
		public:
			using Magic::Magic;

			virtual void OnUpdate() override;

			static bool StartEffect(EffectSetting* effect);
	};
}
