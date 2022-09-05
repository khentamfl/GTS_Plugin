#pragma once
// Module that handles AttributeAdjustment


using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {

	struct SizeManagerData {
		float extraMaxSize = 0.0;
	};
	class SizeManager {
		public:
			[[nodiscard]] static SizeManager& GetSingleton() noexcept;

			void UpdateSize(Actor* Target);

			SizeManagerData& GetData(Actor* actor);

			void SetExtraMaxSize(Actor* actor, float amt);
			float GetExtraMaxSize(Actor* actor);
			void ModExtraMaxSize(Actor* actor, float amt);

		private:
			std::map<Actor*, SizeManagerData> sizeData;
	};
}
