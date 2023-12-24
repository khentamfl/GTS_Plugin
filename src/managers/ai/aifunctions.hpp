#pragma once

#include "events.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace Gts {
	void KillActor(Actor* giant, Actor* tiny);
	void RagdollTask(Actor* tiny);
	void ScareActors(Actor* giant);
	void ScareHostileActors(Actor* giant, float dist);
}