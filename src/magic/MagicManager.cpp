#include "magic/explosive_growth.h"
#include "magic/magic.h"
#include "scale/scale.h"
#include "data/runtime.h"

namespace Gts {
	void SlowGrowthFunction(Actor* caster) // Slow Growth spell
	{
	}

	void GrowthSpellFunction(Actor* caster) // Growth Spell
	{
		auto& runtime = Runtime::GetSingleton();
		float size_limit = runtime.sizeLimit->value;
		float ProgressionMultiplier = runtime.ProgressionMultiplier->value;
		float casterScale = get_visual_scale(caster);
		float DualCast = 1.0;
		if (caster->magicCasters[Actor::SlotTypes::kLeftHand]->GetIsDualCasting())
		{DualCast = 2.0;}
		if (casterScale < size_limit) {
			set_target_scale(caster, casterScale + (0.00125 * ProgressionMultiplier * DualCast));
		}
	}
	void ShrinkSpellFunction(Actor* caster) // Shrink Spell
	{
		auto& runtime = Runtime::GetSingleton();
		float size_limit = runtime.sizeLimit->value;
		float ProgressionMultiplier = runtime.ProgressionMultiplier->value;
		float casterScale = get_visual_scale(caster);
		float DualCast = 1.0;
		if (caster->magicCasters[Actor::SlotTypes::kLeftHand]->GetIsDualCasting())
		{DualCast = 2.0;}
		if (casterScale < size_limit) {
			set_target_scale(caster, casterScale - (0.0018* ProgressionMultiplier * DualCast));
		}
	}

	void GrowAllyFunction(Actor* caster, Actor* target) // Scale Ally with a spell
	{
		auto& runtime = Runtime::GetSingleton();
		float size_limit = runtime.sizeLimit->value;
		float ProgressionMultiplier = runtime.ProgressionMultiplier->value;
		float CrushGrowthRate = runtime.CrushGrowthRate->value;
		float casterScale = get_visual_scale(caster);
		float targetScale = get_visual_scale(target);
		float GrowRate = 0.0;
		float SMTRate = 1.0;
		float DualCast = 1.0;
		if (caster->magicCasters[Actor::SlotTypes::kLeftHand]->GetIsDualCasting())
		{DualCast = 2.0;}
		if (caster->HasMagicEffect(runtime.smallMassiveThreat))
		{SMTRate = 2.0;}
		if (CrushGrowthRate >= 1.4)
		{GrowRate = 0.00180;}

		if (targetScale < size_limit) {
			set_target_scale(target, targetScale * 1.00000 + (((0.00180 + GrowRate) * (casterScale * 0.50 + 0.50) * targetScale) * ProgressionMultiplier * SMTRate * DualCast));
		}
		if (casterScale >= 1.0)
		{set_target_scale(caster, casterScale * 1.00000 - (((0.00180 + GrowRate) * targetScale * 0.50)) * ProgressionMultiplier * SMTRate * DualCast);}
	}

	void ShrinkAllyFunction(Actor* caster, Actor* target) // Shrink Ally with a spell
	{
		auto& runtime = Runtime::GetSingleton();
		float size_limit = runtime.sizeLimit->value;
		float ProgressionMultiplier = runtime.ProgressionMultiplier->value;
		float CrushGrowthRate = runtime.CrushGrowthRate->value;
		float casterScale = get_visual_scale(caster);
		float targetScale = get_visual_scale(target);
		float GrowRate = 0.0;
		float DualCast = 1.0;
		if (caster->magicCasters[Actor::SlotTypes::kLeftHand]->GetIsDualCasting())
		{DualCast = 2.0;}
		if (CrushGrowthRate >= 1.4)
		{GrowRate = 0.00090;}

		if (targetScale > 0.25) {
			set_target_scale(target, targetScale * 1.00000 - (((0.00180 + GrowRate) * (casterScale * 0.50 + 0.50) * targetScale) * ProgressionMultiplier * DualCast));
		}
	}


	void GrowAllyButtonFunction(Actor* target) // Scale Ally with a button press
	{
		auto& runtime = Runtime::GetSingleton();
		float size_limit = runtime.sizeLimit->value;
		float ProgressionMultiplier = runtime.ProgressionMultiplier->value;
		float targetScale = get_visual_scale(target);
		if (targetScale < size_limit)
		{mod_target_scale(target, +((0.0020 * targetScale) * ProgressionMultiplier));}
	}

	void ShrinkAllyButtonFunction(Actor* target) // Shrink Ally with a button press
	{
		auto& runtime = Runtime::GetSingleton();
		float size_limit = runtime.sizeLimit->value;
		float ProgressionMultiplier = runtime.ProgressionMultiplier->value;
		float targetScale = get_visual_scale(target);
		if (targetScale > 1.0)
		{mod_target_scale(target, -((0.0020 * targetScale) * ProgressionMultiplier));}
	}


	void ShrinkBackFunction(Actor* caster) // Restore Size function for PC
	{
		auto& runtime = Runtime::GetSingleton();
		float size_limit = runtime.sizeLimit->value;
		float ProgressionMultiplier = runtime.ProgressionMultiplier->value;
		float casterScale = get_visual_scale(caster);
		if (casterScale < 1.0)
		{set_target_scale(caster, casterScale * 1.0050 + (0.0005 * 10 * ProgressionMultiplier));}
		//else if (casterScale >= 1.01 || casterScale <=1.00)
		//{runtime.ShrinkBackSpell->DispelSpell(caster)}
		else if (casterScale > 1.00)
		{
			set_target_scale(caster, casterScale * 0.9950 - (0.0005 * 10 * ProgressionMultiplier));
		}
	}

	void ShrinkBackNPCFunction(Actor* target) // Restore size but for Followers/NPC's
	{
		auto& runtime = Runtime::GetSingleton();
		float size_limit = runtime.sizeLimit->value;
		float ProgressionMultiplier = runtime.ProgressionMultiplier->value;
		float targetScale = get_visual_scale(target);
		if (targetScale < 1.0)
		{set_target_scale(target, targetScale * 1.0050 + (0.0005 * 10 * ProgressionMultiplier));}
		//else if (targetScale >= 1.01 || targetScale <=1.00)
		//{runtime.ShrinkBackNPCSpell->DispelSpell(target)}
		else if (targetScale > 1.00)
		{
			set_target_scale(target, targetScale * 0.9950 - (0.0005 * 10 * ProgressionMultiplier));
		}
	}

	void VoreGrowthFunction(Actor* caster) // Growth after the Vore
	{
		auto& runtime = Runtime::GetSingleton();
		float size_limit = runtime.sizeLimit->value;
		float ProgressionMultiplier = runtime.ProgressionMultiplier->value;
		float casterScale = get_visual_scale(caster);
		if (casterScale < size_limit)
			set_target_scale(caster, casterScale + ((0.00165) * 0.15 * ProgressionMultiplier));
	}

	void ManageGameModePC()
	{
		auto& runtime = Runtime::GetSingleton();
		float size_limit = runtime.sizeLimit->value;
		float GrowthRate = runtime.GrowthModeRate->value;
		float ShrinkRate = runtime.ShrinkModeRate->value;
		auto Player = PlayerCharacter::GetSingleton();
		float Scale = get_visual_scale(Player);

		if (runtime.ChosenGameMode->value == 1.0 && Scale < size_limit)
		{set_target_scale(Player, Scale * (1.00010 + (GrowthRate * 0.25)));}
		else if (runtime.ChosenGameMode->value == 2.0 && Scale > 1.0)
		{set_target_scale(Player, Scale * (0.99985 - (ShrinkRate * 0.25)));}
		else if (runtime.ChosenGameMode->value == 3.0 && Scale < size_limit)
		{ if (Player->IsInCombat() == true)
		  {set_target_scale(Player, Scale * (1.00008 + (GrowthRate * 0.17)));}
		  else if (Player->IsInCombat() == false)
		  {set_target_scale(Player, Scale * (0.99981 - (ShrinkRate * 0.34)));}}
	}

	void ManageGameModeNPC(Actor* target)
	{
		auto& runtime = Runtime::GetSingleton();
		float size_limit = runtime.sizeLimit->value;
		float GrowthRate = runtime.GrowthModeRateNPC->value;
		float ShrinkRate = runtime.ShrinkModeRateNPC->value;
		float Scale = get_visual_scale(target);

		if (runtime.ChosenGameModeNPC->value == 1.0 && Scale < size_limit)
		{set_target_scale(target, Scale * (1.00010 + (GrowthRate * 0.25)));}
		else if (runtime.ChosenGameModeNPC->value == 2.0 && Scale > 1.0)
		{set_target_scale(target, Scale * (0.99985 - (ShrinkRate * 0.25)));}
		else if (runtime.ChosenGameModeNPC->value == 3.0 && Scale < size_limit)
		{ if (target->IsInCombat() == true)
		  {set_target_scale(target, Scale * (1.00008 + (GrowthRate * 0.17)));}
		  else if (target->IsInCombat() == false)
		  {set_target_scale(target, Scale * (0.99981 - (ShrinkRate * 0.34)));}}
	}

}
