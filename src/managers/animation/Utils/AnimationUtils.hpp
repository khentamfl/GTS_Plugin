#pragma once

#include "events.hpp"
#include "managers/animation/Utils/AttachPoint.hpp"
#include "managers/animation/Utils/TurnTowards.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace Gts {
	void AllowToDoVore(Actor* actor, bool toggle);
	void AllowToBeCrushed(Actor* actor, bool toggle);
	void ManageCamera(Actor* giant, bool enable, float type);

	void DoLaunch(Actor* giant, float radius, float damage, float overr, FootEvent kind);

	void GrabStaminaDrain(Actor* giant, Actor* tiny, float sizedifference);
	void DrainStamina(Actor* giant, std::string_view TaskName, std::string_view perk, bool decide, float waste, float power);

	void SpawnHurtParticles(Actor* giant, Actor* grabbedActor, float mult, float dustmult);

	void ToggleEmotionEdit(Actor* giant, bool allow);
	void AdjustFacialExpression(Actor* giant, int ph, float power, std::string_view type);

	float GetPerkBonus_Basics(Actor* Giant);
	float GetPerkBonus_Thighs(Actor* Giant);

	bool IsHostile(Actor* giant, Actor* tiny);
}
