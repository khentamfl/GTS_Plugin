#pragma once
// Module that handles AttributeAdjustment


using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {

	struct SizeManagerData {
		float enchantmentBonus = 0.0;
	};
	class SizeManager {
		public:
			[[nodiscard]] static SizeManager& GetSingleton() noexcept;

			inline float GetRaceScale(Actor* actor);
			void UpdateSize(Actor* Target);

			SizeManagerData& GetData(Actor* actor);

			void SetEnchantmentBonus(Actor* actor, float amt);
			float GetEnchantmentBonus(Actor* actor);
			void ModEnchantmentBonus(Actor* actor, float amt);

		private:
			std::map<Actor*, SizeManagerData> sizeData;
	};
}
