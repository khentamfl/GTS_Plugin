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
			void OverrideBonus(float Value);
			float GetAttributeBonus(Actor* actor, float Value);
		private:
			float MovementSpeedBonus = 0.0;
			bool BlockMessage = false;
	};
}
