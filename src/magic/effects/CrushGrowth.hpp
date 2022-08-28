#pragma once
#include "magic/magic.hpp"
// Module that tracks the target


using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	class CrushingGrowth : public Magic {
		public:
			using Magic::Magic;

			virtual void OnUpdate() override;

            virtual void OnFinish() override;

			virtual std::string GetName() override;

			bool StartEffect(EffectSetting* effect);

            private:
            float CrushGrowthAmount = 1.0;
	};
}