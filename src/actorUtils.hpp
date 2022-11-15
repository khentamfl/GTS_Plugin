#pragma once

namespace Gts {
	void PlayAnimation(Actor* actor, std::string_view animName);

	void TransferInventory(Actor* from, Actor* to);

	void Disintegrate(Actor* actor);
	void UnDisintegrate(Actor* actor);

	void SetRestrained(Actor* actor);
	void SetUnRestrained(Actor* actor);

	void SetDontMove(Actor* actor);
	void SetMove(Actor* actor);

	void PlayImpactEffect(Actor* actor, BGSImpactDataSet* a_impactEffect, std::string_view node, NiPoint3& direction, float length, bool applyRotation, bool useLocalRotation);

	void KnockAreaEffect(Actor* actor, float afMagnitude, float afRadius);
}
