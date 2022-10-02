#pragma once
// Module that handles AttributeAdjustment
#include "events.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {

	struct SizeManagerData {
		float enchantmentBonus = 0.0;
		float SizeHungerBonus = 0.0;
	};
	class SizeManager : public EventListener {
		public:
			[[nodiscard]] static SizeManager& GetSingleton() noexcept;

			inline float GetRaceScale(Actor* actor);
			virtual void Update() override;

			SizeManagerData& GetData(Actor* actor);

			void SetEnchantmentBonus(Actor* actor, float amt);
			float GetEnchantmentBonus(Actor* actor);
			void ModEnchantmentBonus(Actor* actor, float amt);

			void SetSizeHungerBonus(Actor* actor, float amt);
			float GetSizeHungerBonus(Actor* actor);
			void ModSizeHungerBonus(Actor* actor, float amt);
			

		private:
			std::map<Actor*, SizeManagerData> sizeData;
	};
}
