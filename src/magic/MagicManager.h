#pragma once
#include <SKSE/SKSE.h>

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
  void ShrinkFoe(Actor* caster, Actor* Target);
  void ShrinkFoeAoe(Actor* caster, Actor* Target);
  void ShrinkFoeAoeMast(Actor* caster, Actor* Target);
}
