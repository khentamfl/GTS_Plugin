#pragma once

#include "events.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace RE
{
	class Actor; 

	struct ActorKill_RE
	{
	public:
		struct Event
		{
		public:
			// members
			Actor* killer;  // 00
			Actor* victim;  // 08
		};
		static_assert(sizeof(Event) == 0x10);

		static BSTEventSource<Event>* GetEventSource();
	};
}

namespace Gts {
	void KillActor(Actor* giant, Actor* tiny);
	void ForceCombat(Actor* giant, Actor* tiny);

	float GetGrowthCount(Actor* giant);
	float GetGrowthLimit(Actor* actor);
	float GetButtCrushDamage(Actor* actor);

	void ModGrowthCount(Actor* giant, float value, bool reset);
	void SetBonusSize(Actor* giant, float value, bool reset);
	

	void ScareActors(Actor* giant);
}