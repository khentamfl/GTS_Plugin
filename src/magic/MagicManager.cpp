#include "magic/explosive_growth.h"
#include "magic/magic.h"
#include "scale/scale.h"

namespace Gts {
	
	void ShrinkFoe(Actor* caster, Actor* target) 
  {
    TESGlobal* SizeLimit = find_form<TESGlobal>("GTS.esp|2028B4");
    TESGlobal* progression_multiplier_global = find_form<TESGlobal>("GTS.esp|37E46E");
    float TargetScale = Get_Visual_Scale(Target); 
    float CasterScale = Get_Visual_Scale(Caster);
    SetTargetScale(target, TargetScale * 0.99935)
	    
      if (CasterScale < SizeLimit)
   	 {
	      SetTargetScale(Caster, casterScale + 0.000015)
	 }
  }
  
  void ShrinkFoeAoe(Actor* caster, Actor* target)
  {
    TESGlobal* SizeLimit = find_form<TESGlobal>("GTS.esp|2028B4");
    TESGlobal* progression_multiplier_global = find_form<TESGlobal>("GTS.esp|37E46E");
    float TargetScale = Get_Visual_Scale(Target); 
    float CasterScale = Get_Visual_Scale(Caster);
    SetTargetScale(target, TargetScale * 0.99905) //- ((ShrinkRateTotal * SMTRate) * 1 * Efficiency))
      
    if (CasterScale < SizeLimit)
   	 {
	    SetTargetScale(Caster, casterScale + 0.000020)
    	 }
  }
  
  void ShrinkFoeAoeMast(Actor* caster, Actor* target)
  {
    TESGlobal* SizeLimit = find_form<TESGlobal>("GTS.esp|2028B4");
    TESGlobal* progression_multiplier_global = find_form<TESGlobal>("GTS.esp|37E46E");
    float TargetScale = Get_Visual_Scale(Target); 
    float CasterScale = Get_Visual_Scale(Caster);
    SetTargetScale(target, TargetScale * 0.99705)
      
      if (CasterScale < SizeLimit)
   	 {
	      SetTargetScale(Caster, casterScale + 0.000025)
	 }
  }
			     
 void SwordOfSize(Actor* caster, Actor* target)
  {
    TESGlobal* SizeLimit = find_form<TESGlobal>("GTS.esp|2028B4");
    TESGlobal* progression_multiplier_global = find_form<TESGlobal>("GTS.esp|37E46E");
    float TargetScale = Get_Visual_Scale(Target); 
    float CasterScale = Get_Visual_Scale(Caster);
	  if (TargetScale > 0.10)
	  {SetTargetScale(Target, TargetScale * 1 - 0.00280)}
      
      if (CasterScale < SizeLimit)
   	 {
	      SetTargetScale(Caster, casterScale + 0.00096)
	 }
  }
}
