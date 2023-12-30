#pragma once

#include "events.hpp"
#include "managers/animation/Utils/AttachPoint.hpp"
#include "managers/animation/Utils/TurnTowards.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace Gts {
	void BlockFirstPerson(Actor* actor, bool block);
	void AllowToDoVore(Actor* actor, bool toggle);
	void AllowToBeCrushed(Actor* actor, bool toggle);
	void ManageCamera(Actor* giant, bool enable, float type);

	void DoLaunch(Actor* giant, float radius, float damage, FootEvent kind);
	void DoLaunch(Actor* giant, float radius, float damage, NiAVObject* node);

	void GrabStaminaDrain(Actor* giant, Actor* tiny, float sizedifference);
	void DrainStamina(Actor* giant, std::string_view TaskName, std::string_view perk, bool decide, float waste, float power);

	void SpawnHurtParticles(Actor* giant, Actor* grabbedActor, float mult, float dustmult);

	void ToggleEmotionEdit(Actor* giant, bool allow);
	void AdjustFacialExpression(Actor* giant, int ph, float power, std::string_view type);

	float GetWasteMult(Actor* giant);
	float GetPerkBonus_Basics(Actor* Giant);
	float GetPerkBonus_Thighs(Actor* Giant);

	void DoFootGrind(Actor* giant, Actor* tiny);
	void FootGrindCheck_Left(Actor* actor, float radius, bool strong);
	void FootGrindCheck_Right(Actor* actor, float radius, bool strong);
	void DoDamageAtPoint_Cooldown(Actor* giant, float radius, float damage, NiAVObject* node, float random, float bbmult, float crushmult, float pushpower, DamageSource Cause);

	NiPoint3 GetHeartPosition(Actor* giant, Actor* tiny);
}
