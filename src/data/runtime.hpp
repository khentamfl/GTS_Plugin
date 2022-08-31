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
			BGSSoundDescriptorForm* shrinkSound = nullptr;

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

			EffectSetting* SizeRelatedDamage0 = nullptr;
			EffectSetting* SizeRelatedDamage1 = nullptr;
			EffectSetting* SizeRelatedDamage2 = nullptr;

			EffectSetting* AbsorbMGEF = nullptr;
			EffectSetting* TrueAbsorb = nullptr;
			SpellItem * TrueAbsorbSpell = nullptr;

			///End

			BGSExplosion* footstepExplosion = nullptr;

			BGSPerk* GrowthOnHitPerk = nullptr;
			BGSPerk* hhBonus = nullptr;
			BGSPerk* PerkPart1 = nullptr;
			BGSPerk* PerkPart2 = nullptr;
			BGSPerk* ExtraGrowth = nullptr;
			BGSPerk* ExtraGrowthMax = nullptr;

			TESGlobal * sizeLimit = nullptr;

			TESGlobal * GtsNPCEffectImmunityToggle = nullptr;

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
			EffectSetting* FakeCrushEffect = nullptr;

			MagicItem * ShrinkBackNPCSpell = nullptr;
			MagicItem * ShrinkBackSpell = nullptr;

			///Camera
			TESGlobal * EnableCamera = nullptr;
			TESGlobal * EnableAltCamera = nullptr;
			TESGlobal * FeetCamera = nullptr;
			TESGlobal * usingAutoDistance = nullptr;
			TESGlobal * ImCrouching = nullptr;

			TESGlobal * MinDistance = nullptr;
			TESGlobal * MaxDistance = nullptr;
			TESGlobal * CameraZoomSpeed = nullptr;
			TESGlobal * CameraZoomPrecision = nullptr;

			TESGlobal * proneCameraX = nullptr;
			TESGlobal * proneCameraY = nullptr;
			TESGlobal * proneCombatCameraX = nullptr;
			TESGlobal * proneCombatCameraY = nullptr;

			TESGlobal * cameraX = nullptr;
			TESGlobal * cameraY = nullptr;
			TESGlobal * combatCameraX = nullptr;
			TESGlobal * combatCameraY = nullptr;

			TESGlobal * proneCameraAlternateX = nullptr;
			TESGlobal * proneCameraAlternateY = nullptr;
			TESGlobal * proneCombatCameraAlternateX = nullptr;
			TESGlobal * proneCombatCameraAlternateY = nullptr;

			TESGlobal * cameraAlternateX = nullptr;
			TESGlobal * cameraAlternateY = nullptr;
			TESGlobal * combatCameraAlternateX = nullptr;
			TESGlobal * combatCameraAlternateY = nullptr;

			TESGlobal * CalcProne = nullptr;
			/////////
	};
}
