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
	}
	void ShrinkSpellFunction(Actor* caster) // Shrink Spell
	{

	}

	void GrowAllyFunction(Actor* caster, Actor* target) // Scale Ally with a spell
	{
	}

	void ShrinkAllyFunction(Actor* caster, Actor* target) // Shrink Ally with a spell
	{
	}


	void GrowAllyButtonFunction(Actor* target) // Scale Ally with a button press
	{

	}

	void ShrinkAllyButtonFunction(Actor* target) // Shrink Ally with a button press
	{

	}


	void ShrinkBackFunction(Actor* caster) // Restore Size function for PC
	{

	}

	void ShrinkBackNPCFunction(Actor* target) // Restore size but for Followers/NPC's
	{

	}

	void VoreGrowthFunction(Actor* caster) // Growth after the Vore
	{

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
