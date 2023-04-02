#pragma once

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	struct ThighData {
		ThighData(TESObjectREFR* tiny, bool RuneSpawn, bool RuneDespawn);
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

			static void GrabActor(Actor* giant, TESObjectREFR* tiny);
			static void Release(Actor* giant);
			// Get object being held
			static TESObjectREFR* GetHeldObj(Actor* giant);

			static void RegisterEvents();

			static void RegisterTriggers();
	};
}
