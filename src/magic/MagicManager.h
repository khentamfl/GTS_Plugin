#pragma once
#include <SKSE/SKSE.h>

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
  void Shrink(Actor* caster, Actor* Target);
  void ShrinkAoe(Actor* caster, Actor* Target);
  void ShrinkAoeMast(Actor* caster, Actor* Target);
}
