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

	std::vector<hkpRigidBody*> GetActorRBs(Actor* actor);
	void PushActorAway(TESObjectREFR* source, Actor* receiver, float afKnockbackForce);
	void KnockAreaEffect(TESObjectREFR* source, float afMagnitude, float afRadius);
	void ApplyHavokImpulse(TESObjectREFR* target, float afX, float afY, float afZ, float afMagnitude);


	bool IsDragon(Actor* actor);

	bool IsProne(Actor* actor);

	float get_distance_to_actor(Actor* receiver, Actor* target);

	bool IsJumping(Actor* actor);

	void ApplyShake(Actor* caster, float modifier);
	void ApplyShakeAtNode(Actor* caster, float modifier, std::string_view node);
	void ApplyShakeAtPoint(Actor* caster,float modifier, const NiPoint3& coords);
}
