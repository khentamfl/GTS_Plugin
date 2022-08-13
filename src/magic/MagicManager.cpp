#include "magic/explosive_growth.h"
#include "magic/magic.h"
#include "scale/scale.h"

namespace Gts {
	
	void ShrinkFoe(Actor* caster, Actor* target) 
  {
    TESGlobal* SizeLimit = find_form<TESGlobal>("GTS.esp|2028B4");
    float size_limit = SizeLimit->value;
    float TargetScale = get_visual_scale(target); 
    float casterScale = get_visual_scale(caster);
    set_target_scale(target, TargetScale * 0.99935);
	    
      if (casterScale < size_limit) {
	      set_target_scale(caster, casterScale + 0.000015);
	 }
  }
  
  void ShrinkFoeAoe(Actor* caster, Actor* target)
  {
    TESGlobal* SizeLimit = find_form<TESGlobal>("GTS.esp|2028B4");
    float size_limit = SizeLimit->value;
    float TargetScale = get_visual_scale(target); 
    float casterScale = get_visual_scale(caster);
    set_target_scale(target, TargetScale * 0.99905); //- ((ShrinkRateTotal * SMTRate) * 1 * Efficiency))
      
    if (casterScale < size_limit) {
	    set_target_scale(caster, casterScale + 0.000020);
    	 }
  }
  
  void ShrinkFoeAoeMast(Actor* caster, Actor* target)
  {
    TESGlobal* SizeLimit = find_form<TESGlobal>("GTS.esp|2028B4");
    float size_limit = SizeLimit->value;
    float TargetScale = get_visual_scale(target); 
    float casterScale = get_visual_scale(caster);
    set_target_scale(target, TargetScale * 0.99705);
      
      if (casterScale < size_limit) {
	      set_target_scale(caster, casterScale + 0.000025);
	 }
  }
			     
 void SwordOfSize(Actor* caster, Actor* target)
  {
    TESGlobal* SizeLimit = find_form<TESGlobal>("GTS.esp|2028B4");
    float size_limit = SizeLimit->value;
    float TargetScale = get_visual_scale(target); 
    float casterScale = get_visual_scale(caster);
	  if (TargetScale > 0.10) {
		  set_target_scale(target, TargetScale * 1 - 0.00280);
	  }
      
      if (casterScale < size_limit) {
	      set_target_scale(caster, casterScale + 0.00096);
	 }
  }

  void ShrinkPCFunction(Actor* caster) // <- Shrink in Size button
  {
    TESGlobal* SizeLimit = find_form<TESGlobal>("GTS.esp|2028B4");
    float size_limit = SizeLimit->value;
    float casterScale = get_visual_scale(caster);
	  if (casterScale > 0.25) {
		  mod_target_scale(caster, -0.0025 * casterScale);
	  }
  }

  void GrowPCFunction(Actor* caster) // <- Grow in Size button
  {
    TESGlobal* SizeLimit = find_form<TESGlobal>("GTS.esp|2028B4");
    float size_limit = SizeLimit->value;
    float casterScale = get_visual_scale(caster);
	  if (casterScale < size_limit) {
		  mod_target_scale(caster, 0.0025 * casterScale);
	  }
      
  }

  void SlowGrowthFunction(Actor* caster) // Slow Growth spell
  {
    TESGlobal* SizeLimit = find_form<TESGlobal>("GTS.esp|2028B4");
    TESGlobal* progression_multiplier_global = find_form<TESGlobal>("GTS.esp|37E46E");
    float size_limit = SizeLimit->value;
    float ProgressionMultiplier = progression_multiplier_global->value;
    float casterScale = get_visual_scale(caster);
	  if (casterScale < size_limit) {
		  mod_target_scale(caster, 0.0010 * ProgressionMultiplier);
	  }
  }

  void GrowthSpellFunction(Actor* caster) // Growth Spell
  {
    TESGlobal* SizeLimit = find_form<TESGlobal>("GTS.esp|2028B4");
    TESGlobal* progression_multiplier_global = find_form<TESGlobal>("GTS.esp|37E46E");
    float size_limit = SizeLimit->value;
    float ProgressionMultiplier = progression_multiplier_global->value;
    float casterScale = get_visual_scale(caster);
	  if (casterScale < size_limit) {
		  mod_target_scale(caster, casterScale + (0.00125 * ProgressionMultiplier));
	  }
  }
  void ShrinkSpellFunction(Actor* caster) // Shrink Spell
  {
    TESGlobal* SizeLimit = find_form<TESGlobal>("GTS.esp|2028B4");
    TESGlobal* progression_multiplier_global = find_form<TESGlobal>("GTS.esp|37E46E");
    float size_limit = SizeLimit->value;
    float ProgressionMultiplier = progression_multiplier_global->value;
    float casterScale = get_visual_scale(caster);
	  if (casterScale < size_limit) {
		  mod_target_scale(caster, casterScale - (0.0018* ProgressionMultiplier));
	  }
  }

  void GrowAllyFunction(Actor* caster, Actor* target) // Scale Ally with a spell
  {
    TESGlobal* SizeLimit = find_form<TESGlobal>("GTS.esp|2028B4");
    TESGlobal* progression_multiplier_global = find_form<TESGlobal>("GTS.esp|37E46E");
    float size_limit = SizeLimit->value;
    float ProgressionMultiplier = progression_multiplier_global->value;
    float casterScale = get_visual_scale(caster);
    float targetScale = get_visual_scale(target);
	  if (casterScale < size_limit) {
		  set_target_scale(target, targetScale * 1.00000 + (((0.00180) * (casterScale * 0.50 + 0.50) * targetScale) * ProgressionMultiplier));
	  }
    if (casterScale >= 1.0)
      {set_target_scale(caster, casterScale * 1.00000 - (((0.00180) * targetScale * 0.25)) * ProgressionMultiplier);}
  }


  void GrowAllyButtonFunction(Actor* target) // Scale Ally with a button press
  {
    TESGlobal* SizeLimit = find_form<TESGlobal>("GTS.esp|2028B4");
    TESGlobal* progression_multiplier_global = find_form<TESGlobal>("GTS.esp|37E46E");
    float size_limit = SizeLimit->value;
    float ProgressionMultiplier = progression_multiplier_global->value;
    float targetScale = get_visual_scale(target);
    if (targetScale < size_limit)
    {mod_target_scale(target, + ((0.0020 * targetScale) * ProgressionMultiplier));}
    }

     void ShrinkAllyButtonFunction(Actor* target) // Shrink Ally with a button press
  {
    TESGlobal* SizeLimit = find_form<TESGlobal>("GTS.esp|2028B4");
    TESGlobal* progression_multiplier_global = find_form<TESGlobal>("GTS.esp|37E46E");
    float size_limit = SizeLimit->value;
    float ProgressionMultiplier = progression_multiplier_global->value;
    float targetScale = get_visual_scale(target);
    if (targetScale > 1.0)
    {mod_target_scale(target, - ((0.0020 * targetScale) * ProgressionMultiplier));}
    }


    void ShrinkBackFunction(Actor* caster) // Restore Size function for PC
  {
    TESGlobal* SizeLimit = find_form<TESGlobal>("GTS.esp|2028B4");
    TESGlobal* progression_multiplier_global = find_form<TESGlobal>("GTS.esp|37E46E");
    float size_limit = SizeLimit->value;
    float ProgressionMultiplier = progression_multiplier_global->value;
    float casterScale = get_visual_scale(caster);
    if (casterScale < 1.0)
    {set_target_scale(caster, casterScale * 1.0050 + (0.0005 * 10 * ProgressionMultiplier))}
    
    else if (casterScale > 1.00)
    {
      set_target_scale(caster, casterScale * 0.9950 - (0.0005 * 10 * ProgressionMultiplier))
    }
  }

   void ShrinkBackNPCFunction(Actor* target) // Restore size but for Followers/NPC's
  {
    TESGlobal* SizeLimit = find_form<TESGlobal>("GTS.esp|2028B4");
    TESGlobal* progression_multiplier_global = find_form<TESGlobal>("GTS.esp|37E46E");
    float size_limit = SizeLimit->value;
    float ProgressionMultiplier = progression_multiplier_global->value;
    float targetScale = get_visual_scale(target);
    if (targetScale < 1.0)
    {set_target_scale(target, targetScale * 1.0050 + (0.0005 * 10 * ProgressionMultiplier))}
    
    else if (targetScale > 1.00)
    {
      set_target_scale(target, targetScale * 0.9950 - (0.0005 * 10 * ProgressionMultiplier))
    }
  }

   void VoreGrowthFunction(Actor* caster) // Growth after the Vore
  {
    TESGlobal* SizeLimit = find_form<TESGlobal>("GTS.esp|2028B4");
    TESGlobal* progression_multiplier_global = find_form<TESGlobal>("GTS.esp|37E46E");
    float size_limit = SizeLimit->value;
    float ProgressionMultiplier = progression_multiplier_global->value;
    float casterScale = get_visual_scale(caster);
    if (casterScale < size_limit)
    set_target_scale(caster, casterScale + ((0.00165) * 0.15 * TargetSize * ProgressionMultiplier));
  }
}
