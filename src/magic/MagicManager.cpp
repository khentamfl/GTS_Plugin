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

  void ShrinkPCFunction(Actor* caster)
  {
    TESGlobal* SizeLimit = find_form<TESGlobal>("GTS.esp|2028B4");
    float size_limit = SizeLimit->value;
    float casterScale = get_visual_scale(caster);
	  if (casterScale > 0.25) {
		  mod_target_scale(caster, -0.0025 * casterScale);
	  }
  }

  void GrowPCFunction(Actor* caster)
  {
    TESGlobal* SizeLimit = find_form<TESGlobal>("GTS.esp|2028B4");
    float size_limit = SizeLimit->value;
    float casterScale = get_visual_scale(caster);
	  if (casterScale > 0.25) {
		  mod_target_scale(caster, 0.0025 * casterScale);
	  }
      
  }

  void SlowGrowthFunction(Actor* caster)
  {
    TESGlobal* SizeLimit = find_form<TESGlobal>("GTS.esp|2028B4");
    TESGlobal* progression_multiplier_global = find_form<TESGlobal>("GTS.esp|37E46E");
    float size_limit = SizeLimit->value;
    float ProgressionMultiplier = progression_multiplier_global->value;
    float casterScale = get_visual_scale(caster);
	  if (casterScale > 0.25) {
		  mod_target_scale(caster, 0.00010 * ProgressionMultiplier);
	  }
      
  }

}
