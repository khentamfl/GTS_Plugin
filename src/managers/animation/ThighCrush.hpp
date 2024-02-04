#pragma once

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts
{
	class AnimationThighCrush {
		public:
			static void RegisterEvents();

			static void RegisterTriggers();
			
	};

	void ApplyThighDamage(Actor* actor, bool right, bool CooldownCheck, float radius, float damage, float bbmult, float crush_threshold, int random, DamageSource Cause);
}
