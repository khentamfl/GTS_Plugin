#pragma once
// Module that handles AttributeValues


using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {

	class AttributeManager {
		public:
			[[nodiscard]] static AttributeManager& GetSingleton() noexcept;

			void Update();
			void UpdateNpc(Actor* Npc);
			inline float Augmentation();
			void OverrideBonus(float Value);
			private:
			float MovementSpeedBonus = 0.0;
			bool BlockMessage = false;
	};
}
