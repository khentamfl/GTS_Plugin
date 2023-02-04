#pragma once
// Module that handles AttributeValues
#include "events.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {

	class AttributeManager : public EventListener {
		public:
			[[nodiscard]] static AttributeManager& GetSingleton() noexcept;

			virtual std::string DebugName() override;
			virtual void Update() override;

			void Augmentation();
			void OverrideSMTBonus(float Value);
			float GetAttributeBonus(Actor* actor, float Value);
		private:
			float MovementSpeedBonus = 0.0;
			bool BlockMessage = false;
			SoftPotential getspeed { 
				.k = 0.142, // 0.125
				.n = 0.82, // 0.86
				.s = 1.90, // 1.12
				.o = 1.0,
				.a = 0.0,  //Default is 0
		};
	};
}
