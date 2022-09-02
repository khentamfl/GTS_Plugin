#include "data/runtime.hpp"
#include "Config.hpp"

using namespace SKSE;
using namespace RE;

namespace {
	template <class T>
	T* find_form(std::string_view lookup_id) {
		// From https://github.com/Exit-9B/MCM-Helper/blob/a39b292909923a75dbe79dc02eeda161763b312e/src/FormUtil.cpp
		std::string lookup_id_str(lookup_id);
		std::istringstream ss{ lookup_id_str };
		std::string plugin, id;

		std::getline(ss, plugin, '|');
		std::getline(ss, id);
		RE::FormID relativeID;
		std::istringstream{ id } >> std::hex >> relativeID;
		const auto dataHandler = RE::TESDataHandler::GetSingleton();
		return dataHandler ? dataHandler->LookupForm<T>(relativeID, plugin) : nullptr;
	}
}

namespace Gts {
	Runtime& Runtime::GetSingleton() noexcept {
		static Runtime instance;
		return instance;
	}

	void Runtime::Load() {
		this->lFootstepL = find_form<BGSSoundDescriptorForm>(Config::GetSingleton().GetSound().GetLFootstepL());
		this->lFootstepR = find_form<BGSSoundDescriptorForm>(Config::GetSingleton().GetSound().GetLFootstepR());

		this->lJumpLand = find_form<BGSSoundDescriptorForm>(Config::GetSingleton().GetSound().GetLJumpLand());

		this->xlFootstepL = find_form<BGSSoundDescriptorForm>(Config::GetSingleton().GetSound().GetXLFootstepL());
		this->xlFootstepR = find_form<BGSSoundDescriptorForm>(Config::GetSingleton().GetSound().GetXLFootstepR());

		this->xlRumbleL = find_form<BGSSoundDescriptorForm>(Config::GetSingleton().GetSound().GetXLRumbleL());
		this->xlRumbleR = find_form<BGSSoundDescriptorForm>(Config::GetSingleton().GetSound().GetXLRumbleR());

		this->xlSprintL = find_form<BGSSoundDescriptorForm>(Config::GetSingleton().GetSound().GetXLSprintL());
		this->xlSprintR = find_form<BGSSoundDescriptorForm>(Config::GetSingleton().GetSound().GetXLSprintR());

		this->xxlFootstepL = find_form<BGSSoundDescriptorForm>(Config::GetSingleton().GetSound().GetXXLFootstepL());
		this->xxlFootstepR = find_form<BGSSoundDescriptorForm>(Config::GetSingleton().GetSound().GetXXLFootstepR());

		this->growthSound = find_form<BGSSoundDescriptorForm>("GTS.esp|271EF6");
		this->shrinkSound = find_form<BGSSoundDescriptorForm>("GTS.esp|364F6A");

		this->MoanSound = find_form<BGSSoundDescriptorForm>("GTS.esp|09B0AC");

		this->SmallMassiveThreat = find_form<EffectSetting>(Config::GetSingleton().GetSpellEffects().GetSmallMassiveThreat());
		this->SmallMassiveThreatSizeSteal = find_form<BGSPerk>("GTS.esp|2496E8");
		this->SmallMassiveThreatSpell = find_form<SpellItem>("GTS.esp|1A2566");

		this->explosiveGrowth1 = find_form<EffectSetting>("GTS.esp|007928"); // < Growth Spurt shouts

		this->explosiveGrowth2 = find_form<EffectSetting>("GTS.esp|1E42A5");

		this->explosiveGrowth3 = find_form<EffectSetting>("GTS.esp|1E42A6");
		///Shrink Spells
		this->ShrinkPCButton = find_form<EffectSetting>("GTS.esp|10A6CF"); // <- Shrink PC in size on button press.
		this->ShrinkBack = find_form<EffectSetting>("GTS.esp|005369"); // < - Spell that restores size back to normal.
		this->ShrinkBackNPC = find_form<EffectSetting>("GTS.esp|00536C"); // < - Spell that restores size back to normal, for NPC.
		this->ShrinkSpell = find_form<EffectSetting>("GTS.esp|002850"); // <- Shrink Self over time [Hands]
		this->ShrinkAlly = find_form<EffectSetting>("GTS.esp|0058D1"); // <- Shrink ally without hostile damage and no aggro.

		this->ShrinkEnemy = find_form<EffectSetting>("GTS.esp|00387B");
		this->ShrinkEnemyAOE = find_form<EffectSetting>("GTS.esp|0DCDC5");
		this->ShrinkEnemyAOEMast = find_form<EffectSetting>("GTS.esp|0DCDCA");
		this->ShrinkBolt = find_form<EffectSetting>("GTS.esp|3C5278");
		this->ShrinkStorm = find_form<EffectSetting>("GTS.esp|3C527C");
		this->SwordEnchant = find_form<EffectSetting>("GTS.esp|00FA9E");

		this->ShrinkToNothing = find_form<EffectSetting>("GTS.esp|009979"); // <- Absorbs someone
		///End

		///Ally/Grow Spells
		this->SlowGrowth = find_form<EffectSetting>("GTS.esp|019C3D"); // <- slow growth spell [Hands]. Release and grow over time.
		this->SlowGrowth2H = find_form<EffectSetting>("GTS.esp|086C8D");
		this->GrowthSpell = find_form<EffectSetting>("GTS.esp|0022EB"); // <- Grow Spell [Hands]
		this->GrowPcButton = find_form<EffectSetting>("GTS.esp|002DB5"); // <- Grow PC in size on button press

		this->GrowAlly = find_form<EffectSetting>("GTS.esp|0058D5");  // <- Increase Ally Size [Hands]

		this->GrowAllySizeButton = find_form<EffectSetting>("GTS.esp|123BE3");  // <- Makes ally grow for 2 sec on button press.
		this->ShrinkAllySizeButton = find_form<EffectSetting>("GTS.esp|123BE4"); // <- Makes ally shrink for 2 sec on button press.

		this->CrushGrowthMGEF = find_form<EffectSetting>("GTS.esp|2028B6"); // < Grow on Crush. NPC only. Player triggers CrushGrowth on crushing someone via Crush() function in SP.
		this->GtsMarkAlly = find_form<EffectSetting>("GTS.esp|29F82C"); // < Marks ally when changing game mode
		this->TrackSize = find_form<EffectSetting>("GTS.esp|3B0E75");
		this->CrushGrowthSpell = find_form<SpellItem>("GTS.esp|2028B7");
		this->TrackSizeSpell = find_form<SpellItem>("GTS.esp|3B0E73");
		///End

		///Others
		this->GlobalVoreGrowth = find_form<EffectSetting>("GTS.esp|216CCC"); // < Vore Growth, used for both PC and Followers

		this->SizeRelatedDamage0 = find_form<EffectSetting>("GTS.esp|00A441"); // gtsSizeCloakEffect
		this->SizeRelatedDamage1 = find_form<EffectSetting>("GTS.esp|00A9A8"); // gtsApplySizeEffect
		this->SizeRelatedDamage2 = find_form<EffectSetting>("GTS.esp|00B474"); // gtsApplySprintingSizeEffect

		this->AbsorbMGEF = find_form<EffectSetting>("GTS.esp|00B470");
		this->TrueAbsorb = find_form<EffectSetting>("GTS.esp|22B0D5");
		this->TrueAbsorbSpell = find_form<SpellItem>("GTS.esp22B0D6");
		///End

		this->footstepExplosion = find_form<BGSExplosion>(Config::GetSingleton().GetExplosions().GetFootstepExplosion());
		this->BloodExplosion = find_form<BGSExplosion>("GTS.esp|01CE9D");
		this->BloodFX = find_form<BGSExplosion>("Dawnguard.esm|00E7B4");

		this->GrowthOnHitPerk = find_form<BGSPerk>("GTS.esp|30EE52");
		this->AdditionalAbsorption = find_form<BGSPerk>("GTS.esp|151518");

		this->hhBonus = find_form<BGSPerk>(Config::GetSingleton().GetPerks().GetHHBonus());
		this->PerkPart1 = find_form<BGSPerk>("GTS.esp|16081F");
		this->PerkPart2 = find_form<BGSPerk>("GTS.esp|160820");
		this->ExtraGrowth = find_form<BGSPerk>("GTS.esp|332563");
		this->ExtraGrowthMax = find_form<BGSPerk>("GTS.esp|397972");
		this->HealthRegenPerk = find_form<BGSPerk>("GTS.esp|18E160"); 
		this->GrowthAugmentation = find_form<BGSPerk>("GTS.esp|35AD69");
		this->VorePerkRegeneration = find_form<BGSPerk>("GTS.esp|33C764");
		this->VorePerkGreed = find_form<BGSPerk>("GTS.esp|33C765");
		this->GrowthPerk = find_form<BGSPerk>("GTS.esp|128CF6"); 
		this->NoSpeedLoss = find_form<BGSPerk>("GTS.esp|2E663B");
		

		this->sizeLimit = find_form<TESGlobal>("GTS.esp|2028B4");

		this->GtsNPCEffectImmunityToggle = find_form<TESGlobal>("GTS.esp|271EFA");

		this->ProgressionMultiplier = find_form<TESGlobal>("GTS.esp|37E46E");
		this->CrushGrowthRate = find_form<TESGlobal>("GTS.esp|2028B9");

		this->ChosenGameMode = find_form<TESGlobal>("GTS.esp|2EB74C");
		this->GrowthModeRate = find_form<TESGlobal>("GTS.esp|2028C3");
		this->ShrinkModeRate = find_form<TESGlobal>("GTS.esp|2028C4");

		this->ChosenGameModeNPC = find_form<TESGlobal>("GTS.esp|2EB747");
		this->GrowthModeRateNPC = find_form<TESGlobal>("GTS.esp|2EB74B");
		this->ShrinkModeRateNPC = find_form<TESGlobal>("GTS.esp|2EB74A");

		this->GlobalMaxSizeCalc = find_form<TESGlobal>("GTS.esp|20CAC5");
		this->MassBasedSizeLimit = find_form<TESGlobal>("GTS.esp|277005");
		this->SelectedSizeFormula = find_form<TESGlobal>("GTS.esp|277004");

		this->ProtectEssentials = find_form<TESGlobal>("GTS.esp|23A3E2");
		this->EnableGiantSounds = find_form<TESGlobal>("GTS.esp|26CDF0");
		this->PCAdditionalEffects = find_form<TESGlobal>("GTS.esp|3D4586");
		this->NPCSizeEffects = find_form<TESGlobal>("GTS.esp|3D4587");
		this->CrushGrowthStorage = find_form<TESGlobal>("GTS.esp|3D4588");


		///Camera
		this->EnableCamera = find_form<TESGlobal>("GTS.esp|290512");
		this->EnableAltCamera = find_form<TESGlobal>("GTS.esp|290513");
		this->FeetCamera = find_form<TESGlobal>("GTS.esp|290525");
		this->usingAutoDistance = find_form<TESGlobal>("GTS.esp|290524");
		this->ImCrouching = find_form<TESGlobal>("GTS.esp|2C8039");

		this->MinDistance = find_form<TESGlobal>("GTS.esp|29051E");
		this->MaxDistance = find_form<TESGlobal>("GTS.esp|29051F");
		this->CameraZoomSpeed = find_form<TESGlobal>("GTS.esp|290526");
		this->CameraZoomPrecision = find_form<TESGlobal>("GTS.esp|290527");

		this->proneCameraX = find_form<TESGlobal>("GTS.esp|290510");
		this->proneCameraY = find_form<TESGlobal>("GTS.esp|290511");
		this->proneCombatCameraX = find_form<TESGlobal>("GTS.esp|290514");
		this->proneCombatCameraY = find_form<TESGlobal>("GTS.esp|290515");

		this->cameraX = find_form<TESGlobal>("GTS.esp|29051A");
		this->cameraY = find_form<TESGlobal>("GTS.esp|29051B");
		this->combatCameraX = find_form<TESGlobal>("GTS.esp|29051D");
		this->combatCameraY = find_form<TESGlobal>("GTS.esp|29051C");

		this->proneCameraAlternateX = find_form<TESGlobal>("GTS.esp|290516");
		this->proneCameraAlternateY = find_form<TESGlobal>("GTS.esp|290517");
		this->proneCombatCameraAlternateX = find_form<TESGlobal>("GTS.esp|290518");
		this->proneCombatCameraAlternateY = find_form<TESGlobal>("GTS.esp|290519");

		this->cameraAlternateX = find_form<TESGlobal>("GTS.esp|290520");
		this->cameraAlternateY = find_form<TESGlobal>("GTS.esp|290521");
		this->combatCameraAlternateX = find_form<TESGlobal>("GTS.esp|290522");
		this->combatCameraAlternateY = find_form<TESGlobal>("GTS.esp|290523");

		this->CalcProne = find_form<TESGlobal>("GTS.esp|2D733A");
		////////////
		/////Attributes//////
		this->AllowTimeChange = find_form<TESGlobal>("GTS.esp|277001"); // <- Speed AV modification toggler 
		this->bonusHPMultiplier = find_form<TESGlobal>("GTS.esp|28B408");
		this->bonusCarryWeightMultiplier = find_form<TESGlobal>("GTS.esp|28B407");
		this->bonusJumpHeightMultiplier = find_form<TESGlobal>("GTS.esp|28B40A");
		this->bonusDamageMultiplier = find_form<TESGlobal>("GTS.esp|28B409");
		this->bonusSpeedMultiplier = find_form<TESGlobal>("GTS.esp|28B40B");
		this->bonusSpeedMax = find_form<TESGlobal>("GTS.esp|28B40C");

		///EndAttributes///

		///Potions///
		this->EffectGrowthPotion = find_form<EffectSetting>("GTS.esp|3D4582");
		this->ResistShrinkPotion = find_form<EffectSetting>("GTS.esp|3D4583");
		this->EffectSizePotionWeak = find_form<EffectSetting>("GTS.esp|3E38BB");
		this->EffectSizePotionNormal = find_form<EffectSetting>("GTS.esp|3E38BC");
		this->EffectSizePotionStrong = find_form<EffectSetting>("GTS.esp|3E38BD");
		this->EffectSizePotionExtreme = find_form<EffectSetting>("GTS.esp|3E38C0");
		///End Potions///

		this->ShrinkToNothingSpell = find_form<SpellItem>("GTS.esp|00997A");
		this->FakeCrushSpell = find_form<SpellItem>("GTS.esp|271EF7");
		this->FakeCrushEffect = find_form<EffectSetting>("GTS.esp|271EF9");

		this->ShrinkBackNPCSpell = find_form<SpellItem>("GTS.esp|00536B");
		this->ShrinkBackSpell = find_form<SpellItem>("GTS.esp|005368");

		this->MainQuest = find_form<TESQuest>("GTS.esp|005E3A");
	}
}
