#pragma once
#include "magic/magic.hpp"
// Module that handles footsteps


using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	class VoreGrowth : public Magic {
		public:
			using Magic::Magic;

			virtual void OnUpdate() override;

			virtual void OnStart() override;

			virtual void OnFinish() override;

			virtual std::string GetName() override;

			void VoreRegeneration(Actor* Caster);
			void VoreBuffAttributes();

		private:

			float ScaleOnVore = 1.0;
			bool BlockVoreMods = false;

	};
}
