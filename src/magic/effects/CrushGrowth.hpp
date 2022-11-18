#pragma once
#include "magic/magic.hpp"
// Module that tracks the target


using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	class CrushGrowth : public Magic {
		public:
			using Magic::Magic;

			virtual void OnStart() override;

			virtual void OnUpdate() override;

			virtual void OnFinish() override;

			virtual std::string GetName() override;

		private:
			float CrushGrowthAmount = 0.0;
			float ScaleOnCrush = 1.0;
	};
}
