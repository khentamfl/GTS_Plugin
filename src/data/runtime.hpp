#pragma once
// Module that holds data that is loaded at runtime
// This includes various forms


using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	class Runtime {
		public:
			[[nodiscard]] static Runtime& GetSingleton() noexcept;

			void Load();

			BGSSoundDescriptorForm* lFootstepL = nullptr;
			BGSSoundDescriptorForm* lFootstepR = nullptr;

			BGSSoundDescriptorForm* lJumpLand = nullptr;

			BGSSoundDescriptorForm* xlFootstepL = nullptr;
			BGSSoundDescriptorForm* xlFootstepR = nullptr;

			BGSSoundDescriptorForm* xlRumbleL = nullptr;
			BGSSoundDescriptorForm* xlRumbleR = nullptr;

			BGSSoundDescriptorForm* xlSprintL = nullptr;
			BGSSoundDescriptorForm* xlSprintR = nullptr;

			BGSSoundDescriptorForm* xxlFootstepL = nullptr;
			BGSSoundDescriptorForm* xxlFootstepR = nullptr;

			BGSSoundDescriptorForm* growthSound = nullptr;

			EffectSetting* smallMassiveThreat = nullptr;

			EffectSetting* explosiveGrowth1 = nullptr;

			EffectSetting* explosiveGrowth2 = nullptr;

			EffectSetting* explosiveGrowth3 = nullptr;

			///Shrink Effects Start
			EffectSetting* ShrinkPCButton = nullptr;
			EffectSetting* ShrinkBack = nullptr;
			EffectSetting* ShrinkBackNPC = nullptr;
			EffectSetting* ShrinkSpell = nullptr;
			EffectSetting* ShrinkAlly = nullptr;

			EffectSetting* ShrinkEnemy = nullptr;
			EffectSetting* ShrinkEnemyAOE = nullptr;
			EffectSetting* ShrinkEnemyAOEMast = nullptr;
			EffectSetting* SwordEnchant = nullptr;

			EffectSetting* ShrinkToNothing = nullptr;
			///End

			///Ally/Grow Spells
			EffectSetting* SlowGrowth = nullptr;
			EffectSetting* GrowthSpell = nullptr;
			EffectSetting* GrowPcButton = nullptr;

			EffectSetting* GrowAlly = nullptr;
			EffectSetting* GrowAllySizeButton = nullptr;
			EffectSetting* ShrinkAllySizeButton = nullptr;
			EffectSetting* AllyCrushGrowth = nullptr;
			EffectSetting* GtsMarkAlly = nullptr;
			///end

			///Others
			EffectSetting* GlobalVoreGrowth = nullptr;
			///End

			BGSExplosion* footstepExplosion = nullptr;

			BGSPerk* GrowthOnHitPerk = nullptr;
			BGSPerk* hhBonus = nullptr;
			BGSPerk* PerkPart1 = nullptr;
			BGSPerk* PerkPart2 = nullptr;
			BGSPerk* ExtraGrowth = nullptr;
			BGSPerk* ExtraGrowthMax = nullptr;

			TESGlobal * sizeLimit = nullptr;
			TESGlobal * ProgressionMultiplier = nullptr;
			TESGlobal * CrushGrowthRate = nullptr;
			TESGlobal * ChosenGameMode = nullptr;
			TESGlobal * GrowthModeRate = nullptr;
			TESGlobal * ShrinkModeRate = nullptr;

			TESGlobal * ChosenGameModeNPC = nullptr;
			TESGlobal * GrowthModeRateNPC = nullptr;
			TESGlobal * ShrinkModeRateNPC = nullptr;

			TESGlobal * ProtectEssentials = nullptr;

			SpellItem * ShrinkToNothingSpell = nullptr;
			SpellItem * FakeCrushSpell = nullptr;

			MagicItem * ShrinkBackNPCSpell = nullptr;
			MagicItem * ShrinkBackSpell = nullptr;

	};
}
