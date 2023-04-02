#pragma once

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	struct ThighData {
		ThighData(TESObjectRERF* tiny, bool RuneSpawn, bool RuneDespawn);
		Spring RuneController;
		TESObjectREFR* tiny;
		bool RuneSpawn;
		bool RuneDespawn;
	};

	class AnimationThighSandwich : public EventListener {
		public:
			[[nodiscard]] static AnimationThighSandwich& GetSingleton() noexcept;

			virtual std::string DebugName() override;
			virtual void Update() override;
			
			static void RegisterEvents();

			static void RegisterTriggers();
	};
}
