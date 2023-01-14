#pragma once

using namespace RE;
using namespace SKSE;

namespace Gts {
	void PlayAnimation(Actor* actor, std::string_view animName);

	void TransferInventory(Actor* from, Actor* to, bool keepOwnership, bool removeQuestItems);

	void Disintegrate(Actor* actor);
	void UnDisintegrate(Actor* actor);

	void SetRestrained(Actor* actor);
	void SetUnRestrained(Actor* actor);

	void SetDontMove(Actor* actor);
	void SetMove(Actor* actor);


	void PushActorAway(TESObjectREFR* source, TESObjectREFR* receiver, float afKnockbackForce);
	void KnockAreaEffect(TESObjectREFR* source, float afMagnitude, float afRadius);


	bool IsDragon(Actor* actor);

	bool IsProne(Actor* actor);
}
