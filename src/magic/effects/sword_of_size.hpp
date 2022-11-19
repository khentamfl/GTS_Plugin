#pragma once
#include "magic/magic.hpp"
// Module that handles footsteps


using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	class SwordOfSize : public Magic {
		public:
			virtual void OnUpdate() override;

			virtual void OnFinish() override;

			virtual std::string GetName() override;

			SwordOfSize(ActiveEffect* effect);

	};
}
