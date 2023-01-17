#pragma once
#include "magic/magic.hpp"
#include "timer.hpp"
// Module that handles Size related damage


using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	class SizeDamage : public Magic {
		public:
			using Magic::Magic;

			virtual void OnUpdate() override;

			virtual std::string GetName() override;

			void SmallMassiveThreatModification(Actor* Caster, Actor* Target);

		private:
			bool AllowOneShot = false;
			Timer crushtimer = Timer(0.035);
	};
}
