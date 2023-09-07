#pragma once

#include "events.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace Gts {
	void KillActor(Actor* giant, Actor* tiny);
	void RagdollTask(Actor* tiny);
	void StartCombat(Actor* giant, Actor* tiny, bool Forced);
	void ScareActors(Actor* giant);
	void ReportCrime(Actor* giant, Actor* tiny, float value, bool combat);
}