#pragma once
#include "magic/magic.hpp"
// Module that handles Size related damage


using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	class SizeDamage : public Magic {
		public:
			using Magic::Magic;

			virtual void OnUpdate() override;

			static bool StartEffect(EffectSetting* effect);
	};
}
