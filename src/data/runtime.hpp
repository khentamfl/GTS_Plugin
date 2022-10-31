#pragma once
// Module that holds data that is loaded at runtime
// This includes various forms
#include "events.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	class Runtime : public EventListener {
		public:
			[[nodiscard]] static Runtime& GetSingleton() noexcept;

			virtual void DataReady() override;

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

			BGSSoundDescriptorForm* MoanSound = nullptr;
			BGSSoundDescriptorForm* LaughSound = nullptr;

			BGSSoundDescriptorForm* BloodGushSound = nullptr;

			EffectSetting* SmallMassiveThreat = nullptr;
			BGSPerk* SmallMassiveThreatSizeSteal = nullptr;

			EffectSetting* explosiveGrowth1 = nullptr;

			EffectSetting* explosiveGrowth2 = nullptr;

			EffectSetting* explosiveGrowth3 = nullptr;

			///Shrink Effects Start
			EffectSetting* ShrinkPCButton = nullptr;
			EffectSetting* ShrinkBack = nullptr;
			EffectSetting* ShrinkBackNPC = nullptr;
			EffectSetting* ShrinkSpell = nullptr;
			EffectSetting* ShrinkAlly = nullptr;
			EffectSetting* ShrinkAllyAdept = nullptr;
			EffectSetting* ShrinkAllyExpert = nullptr;

			EffectSetting* ShrinkEnemy = nullptr;
			EffectSetting* ShrinkEnemyAOE = nullptr;
			EffectSetting* ShrinkEnemyAOEMast = nullptr;
			EffectSetting* ShrinkBolt = nullptr;
			EffectSetting* ShrinkStorm = nullptr;
			EffectSetting* SwordEnchant = nullptr;
			EffectSetting* EnchGigantism = nullptr;

			EffectSetting* ShrinkToNothing = nullptr;
			SpellItem* SmallMassiveThreatSpell = nullptr;
			///End

			///Ally/Grow Spells
			EffectSetting* SlowGrowth = nullptr;
			EffectSetting* SlowGrowth2H = nullptr;
			EffectSetting* GrowthSpell = nullptr;
			EffectSetting* GrowPcButton = nullptr;

			EffectSetting* GrowAlly = nullptr;
			EffectSetting* GrowAllyAdept = nullptr;
			EffectSetting* GrowAllyExpert = nullptr;
			EffectSetting* GrowAllySizeButton = nullptr;
			EffectSetting* ShrinkAllySizeButton = nullptr;
			EffectSetting* CrushGrowthMGEF = nullptr;
			EffectSetting* GtsMarkAlly = nullptr;
			EffectSetting* TrackSize = nullptr;
			SpellItem* CrushGrowthSpell = nullptr;
			SpellItem* TrackSizeSpell = nullptr;
			///end

			///Others
			EffectSetting* GlobalVoreGrowth = nullptr;

			EffectSetting* SizeRelatedDamage0 = nullptr;
			EffectSetting* SizeRelatedDamage1 = nullptr;
			EffectSetting* SizeRelatedDamage2 = nullptr;

			EffectSetting* AbsorbMGEF = nullptr;
			EffectSetting* TrueAbsorb = nullptr;
			SpellItem * TrueAbsorbSpell = nullptr;

			BGSPerk* VorePerk = nullptr;

			SpellItem * StartVoreFake = nullptr;
			SpellItem * StartVore = nullptr;

			///End

			BGSExplosion* footstepExplosion = nullptr;
			BGSExplosion* BloodExplosion = nullptr;
			BGSExplosion* BloodFX = nullptr;

			BGSPerk* GrowthOnHitPerk = nullptr;
			BGSPerk* AdditionalAbsorption = nullptr;
			BGSPerk* StaggerImmunity = nullptr;
			BGSPerk* hhBonus = nullptr;
			BGSPerk* PerkPart1 = nullptr;
			BGSPerk* PerkPart2 = nullptr;
			BGSPerk* ExtraGrowth = nullptr;
			BGSPerk* ExtraGrowthMax = nullptr;
			BGSPerk* HealthRegenPerk = nullptr;
			BGSPerk* GrowthAugmentation = nullptr;
			BGSPerk* VorePerkRegeneration = nullptr;
			BGSPerk* VorePerkGreed = nullptr;
			BGSPerk* GrowthPerk = nullptr;
			BGSPerk* TotalControl = nullptr;
			BGSPerk* NoSpeedLoss = nullptr;
			BGSPerk* SizeReserve = nullptr;
			BGSPerk* BonusSpeedPerk = nullptr;
			BGSPerk* OnTheEdge = nullptr;

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
			TESGlobal * GlobalMaxSizeCalc = nullptr;
			TESGlobal * MassBasedSizeLimit = nullptr;
			TESGlobal * SelectedSizeFormula = nullptr;

			TESGlobal * ProtectEssentials = nullptr;
			TESGlobal * EnableGiantSounds = nullptr;
			TESGlobal * PCAdditionalEffects = nullptr;
			TESGlobal * NPCSizeEffects = nullptr;
			TESGlobal * CrushGrowthStorage = nullptr;
			TESGlobal * IsFalling = nullptr;
			

			TESGlobal * ManualGrowthStorage = nullptr;
			TESGlobal * BalanceMode = nullptr;
			TESGlobal * HighHeelDamage = nullptr;


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

			/////Attributes//////
			TESGlobal * AllowTimeChange = nullptr;
			TESGlobal * bonusHPMultiplier = nullptr;
			TESGlobal * bonusCarryWeightMultiplier = nullptr;
			TESGlobal * bonusJumpHeightMultiplier = nullptr;
			TESGlobal * bonusDamageMultiplier = nullptr;
			TESGlobal * bonusSpeedMultiplier = nullptr;
			TESGlobal * bonusSpeedMax = nullptr;

			///EndAttributes///

			///Potions///
			EffectSetting* EffectGrowthPotion = nullptr;
			EffectSetting* ResistShrinkPotion = nullptr;
			EffectSetting* EffectSizePotionWeak = nullptr;
			EffectSetting* EffectSizePotionNormal = nullptr;
			EffectSetting* EffectSizePotionStrong = nullptr;
			EffectSetting* EffectSizePotionExtreme = nullptr;
			EffectSetting* EffectSizeHungerPotion = nullptr;
			///End Potions///

			///Size-Damage spells
			SpellItem* gtsSizeCloakSpellTiny = nullptr;
 			SpellItem* gtsSizeCloakSpellSmall = nullptr;
 			SpellItem* gtsSizeCloakSpellMedium = nullptr;
 			SpellItem* gtsSizeCloakSpellLarge = nullptr;
 			SpellItem* gtsSizeCloakSpellHuge = nullptr;
			SpellItem* gtsSizeCloakSpellMega = nullptr;
 			SpellItem* gtsSizeCloakSpellMassive = nullptr;
 			SpellItem* gtsSizeCloakSpellGigantic = nullptr;
 			SpellItem* gtsSizeCloakSpellImpossible = nullptr;
			///End Size-Damage spells


			SpellItem * ShrinkToNothingSpell = nullptr;
			SpellItem * FakeCrushSpell = nullptr;
			EffectSetting* FakeCrushEffect = nullptr;

			SpellItem * ShrinkBackNPCSpell = nullptr;
			SpellItem * ShrinkBackSpell = nullptr;

			TESQuest * MainQuest = nullptr;

			TESFaction* FollowerFaction = nullptr;

	};
}
