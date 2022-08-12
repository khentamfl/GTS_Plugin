#pragma once
#include <SKSE/SKSE.h>

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
  void ShrinkFoe(Actor* caster, Actor* target);
  void ShrinkFoeAoe(Actor* caster, Actor* target);
  void ShrinkFoeAoeMast(Actor* caster, Actor* target);
}
