#pragma once
#include <SKSE/SKSE.h>

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
  void ShrinkFoe(Actor* caster, Actor* target);
  void ShrinkFoeAoe(Actor* caster, Actor* target);
  void ShrinkFoeAoeMast(Actor* caster, Actor* target);
  void SwordOfSize(Actor* caster, Actor* target);
  void ShrinkPCFunction(Actor* caster);
  void GrowPCFunction(Actor* caster);
  void SlowGrowthFunction(Actor* caster);
  void GrowthSpellFunction(Actor* caster);
  void ShrinkSpellFunction(Actor* caster);
  void GrowAllyFunction(Actor* caster, Actor* target);
  void GrowAllyButtonFunction(Actor* target);
  void ShrinkAllyButtonFunction(Actor* target);
  void ShrinkBackFunction(Actor* caster);
  void ShrinkBackNPCFunction(Actor* target);
  void VoreGrowthFunction(Actor* target);
}
