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
		float HitGrowth = 0.0;
		float GrowthTimer = 0.0;
		float GrowthSpurt = 0.0;
		float CrushGrowthStacks = 0.0;
		float VoreStacks = 0.0;

		float NormalDamage = 1.0; // 0
		float SprintDamage = 1.0; // 1
		float FallDamage = 1.0; // 2
		float HHDamage = 1.0; // 3
		float SizeVulnerability = 0.0;

		bool OffsetCamera = false;
		bool IsThighSandwiching = false;
		bool AlterSandwichCamera = false;
		bool ShouldTrackHand = false;
		bool IsVoring = false;

	};

	struct LaunchData {
		double lastLaunchTime = -1.0e8; //
	};

	struct DamageData {
		double lastDamageTime = -1.0e8; //
	};

	class SizeManager : public EventListener {
		public:
			[[nodiscard]] static SizeManager& GetSingleton() noexcept;

			float GetRaceScale(Actor* actor);

			virtual std::string DebugName() override;
			virtual void DataReady() override;
			virtual void Update() override;
			virtual void OnHighheelEquip(const HighheelEquip& evt) override;

			void Reset();

			SizeManagerData& GetData(Actor* actor);

			void SetEnchantmentBonus(Actor* actor, float amt);
			float GetEnchantmentBonus(Actor* actor);
			void ModEnchantmentBonus(Actor* actor, float amt);

			void SetSizeHungerBonus(Actor* actor, float amt);
			float GetSizeHungerBonus(Actor* actor);
			void ModSizeHungerBonus(Actor* actor, float amt);

			void SetGrowthSpurt(Actor* actor, float amt);
			float GetGrowthSpurt(Actor* actor);
			void ModGrowthSpurt(Actor* actor, float amt);

			void SetSizeAttribute(Actor* actor, float amt, float attribute);
			float GetSizeAttribute(Actor* actor, float attribute);
			void ModSizeAttribute(Actor* actor, float amt, float attribute);

			void SetSizeVulnerability(Actor* actor, float amt);
			float GetSizeVulnerability(Actor* actor);
			void ModSizeVulnerability(Actor* actor, float amt);

			float GetHitGrowth(Actor* actor);
			void SetHitGrowth(Actor* actor, float allow);

			void SetActionBool(Actor* actor, bool enable, float type);
			bool GetActionBool(Actor* actor, float type);

			float BalancedMode();

			LaunchData& GetLaunchData(Actor* actor);
			DamageData& GetDamageData(Actor* actor);

			static bool IsLaunching(Actor* actor);
			static bool IsDamaging(Actor* actor);

			bool GetPreciseDamage();

		private:
			std::map<Actor*, SizeManagerData> sizeData;
			std::map<Actor*, LaunchData> launchData;
			std::map<Actor*, DamageData> DamageData;
	};
}
