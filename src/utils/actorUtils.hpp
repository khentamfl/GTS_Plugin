#pragma once

#include "events.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace Gts {
	Actor* GetActorPtr(Actor* actor);

	Actor* GetActorPtr(Actor& actor);

	Actor* GetActorPtr(ActorHandle& actor);

	Actor* GetActorPtr(const ActorHandle& actor);

	Actor* GetActorPtr(FormID formId);

	void PlayAnimation(Actor* actor, std::string_view animName);

	void TransferInventory(Actor* from, Actor* to, bool keepOwnership, bool removeQuestItems);

	void Disintegrate(Actor* actor);
	void UnDisintegrate(Actor* actor);

	void SetRestrained(Actor* actor);
	void SetUnRestrained(Actor* actor);

	void SetDontMove(Actor* actor);
	void SetMove(Actor* actor);

	void ForceRagdoll(Actor* actor, bool forceOn);

	std::vector<hkpRigidBody*> GetActorRBs(Actor* actor);
	void PushActorAway(TESObjectREFR* source, Actor* receiver, float afKnockbackForce);
	void KnockAreaEffect(TESObjectREFR* source, float afMagnitude, float afRadius);
	void ApplyHavokImpulse(Actor* target, float afX, float afY, float afZ, float afMagnitude);

	void CompleteDragonQuest();

	bool AllowStagger(Actor* giant, Actor* tiny);
	bool IsHuman(Actor* actor);
	bool IsDragon(Actor* actor);
	bool IsLiving(Actor* actor);

	bool IsEquipBusy(Actor* actor);

	bool IsProne(Actor* actor);

	float get_distance_to_actor(Actor* receiver, Actor* target);

	bool IsJumping(Actor* actor);

	void ApplyShake(Actor* caster, float modifier);
	void ApplyShakeAtNode(Actor* caster, float modifier, std::string_view node);
	void ApplyShakeAtNode(Actor* caster, float modifier, std::string_view node, float radius);
	void ApplyShakeAtPoint(Actor* caster, float modifier, const NiPoint3& coords, float radius);
	void EnableFreeCamera();

	bool AllowDevourment();
	bool AllowFeetTracking();
	bool LessGore();
	bool IsBeingHeld(Actor* tiny);
	void SetBeingHeld(Actor* tiny, bool decide);

	bool IsBetweenBreasts(Actor* actor);
	void SetBetweenBreasts(Actor* actor, bool decide);

	void SetBeingEaten(Actor* tiny, bool decide);
	bool IsBeingEaten(Actor* tiny);

	bool IsGtsBusy(Actor* actor);
	bool IsTeammate(Actor* actor);

	void TrackFeet(Actor* giant, float number, bool enable);
	void CallDevourment(Actor* giant, Actor* tiny);
	void CallGainWeight(Actor* giant, float value);
	void CallVampire();
	void CallHelpMessage();
	void AddCalamityPerk();
	void PerkPointCheck(float level);
	float GetRandomBoost();

	void DoSizeEffect(Actor* giant, float modifier, FootEvent kind, std::string_view node);
	void SpawnParticle(Actor* actor, float lifetime, const char* modelName, const NiMatrix3& rotation, const NiPoint3& position, float scale, std::uint32_t flags, NiAVObject* target);
	void SpawnDustParticle(Actor* giant, Actor* tiny, std::string_view node, float size);
	void ShutUp(Actor* actor);
	void StaggerOr(Actor* giant, Actor* tiny, float power, float afX, float afY, float afZ, float afMagnitude);
	void DoDamageEffect(Actor* giant, float damage, float radius, int random, float bonedamage);
	bool HasSMT(Actor* giant);
	void TiredSound(Actor* player, std::string_view message);

	hkaRagdollInstance* GetRagdoll(Actor* actor);

	void ManageRagdoll(Actor* tinyref, float deltaLength, NiPoint3 deltaLocation, NiPoint3 targetLocation);
	void StaggerActor(Actor* receiver);

	float GetMovementModifier(Actor* giant);
	float GetXpBonus();

	void AddSMTDuration(Actor* actor, float duration);
	void AddSMTPenalty(Actor* actor, float penalty);

	void PrintDeathSource(Actor* giant, Actor* tiny, std::string_view cause);
	void PrintSuffocate(Actor* pred, Actor* prey);
	void ShrinkUntil(Actor* giant, Actor* tiny, float expected);
	void DisableCollisions(Actor* actor, TESObjectREFR* otherActor);
	void EnableCollisions(Actor* actor);

 
	void SpringGrow(Actor* actor, float amt, float halfLife, std::string_view naming);
	void SpringShrink(Actor* actor, float amt, float halfLife, std::string_view naming);
	void SpringGrow_Free(Actor* actor, float amt, float halfLife, std::string_view naming);
  	void SpringShrink_Free(Actor* actor, float amt, float halfLife, std::string_view naming);
}
