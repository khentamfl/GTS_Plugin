#include "magic/explosive_growth.h"
#include "magic/magic.h"
#include "scale/scale.h"

namespace Gts {
	
	void ShrinkFoe(Actor* caster, Actor* target) 
  {
    TESGlobal* SizeLimit = find_form<TESGlobal>("GTS.esp|2028B4");
    TESGlobal* progression_multiplier_global = find_form<TESGlobal>("GTS.esp|37E46E");
    float TargetScale = GetVisualScale(Target); float CasterScale = GetVisualScale(Caster);
    SetTargetScale(OurTarget, TargetScale * 0.99935)
      if (CasterScale < SizeLimit)
    {SetTargetScale(Caster, casterScale + 0.000015)}
  }
  
  void ShrinkFoeAoe(Actor* caster, Actor* target)
  {
    TESGlobal* SizeLimit = find_form<TESGlobal>("GTS.esp|2028B4");
    TESGlobal* progression_multiplier_global = find_form<TESGlobal>("GTS.esp|37E46E");
    float TargetScale = GetVisualScale(Target); float CasterScale = GetVisualScale(Caster);
    SetTargetScale(OurTarget, TargetScale * 0.99905) //- ((ShrinkRateTotal * SMTRate) * 1 * Efficiency))
      
    if (CasterScale < SizeLimit)
    {SetTargetScale(Caster, casterScale + 0.000020)}
  }
  
  void ShrinkFoeAoeMast(Actor* caster, Actor* target)
  {
    TESGlobal* SizeLimit = find_form<TESGlobal>("GTS.esp|2028B4");
    TESGlobal* progression_multiplier_global = find_form<TESGlobal>("GTS.esp|37E46E");
    float TargetScale = GetVisualScale(Target); float CasterScale = GetVisualScale(Caster);
    SetTargetScale(OurTarget, TargetScale * 0.99705)
      
      if (CasterScale < SizeLimit)
    {SetTargetScale(Caster, casterScale + 0.000025}
  }
}
