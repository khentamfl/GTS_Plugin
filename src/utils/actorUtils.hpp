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

	float GetLaunchPower(float sizeRatio);


	// GTS State Bools
	bool IsEquipBusy(Actor* actor);
	bool IsFootGrinding(Actor* actor);
	bool IsCrawling(Actor* actor);
	bool IsJumping(Actor* actor);
	bool IsBeingHeld(Actor* tiny);
	bool IsBetweenBreasts(Actor* actor);
	bool IsTransferingTiny(Actor* actor);
	bool IsUsingThighAnimations(Actor* actor);
	bool IsSynced(Actor* actor);
	bool CanDoPaired(Actor* actor);
	bool IsThighCrushing(Actor* actor);
	bool IsThighSandwiching(Actor* actor);
	bool IsStomping(Actor* actor);
	bool IsBeingEaten(Actor* tiny);
	bool IsGtsBusy(Actor* actor);
	bool IsCameraEnabled(Actor* actor);
	bool IsCrawlVoring(Actor* actor);
	bool IsButtCrushing(Actor* actor);
	bool ButtCrush_IsAbleToGrow(Actor* actor, float limit);
	bool IsBeingGrinded(Actor* actor);
	bool CanDoButtCrush(Actor* actor);
	bool GetCameraOverride(Actor* actor);
    // GTS State Bools End

    // Gts Bools
	bool IsGrowthSpurtActive(Actor* actor);
	bool HasGrowthSpurt(Actor* actor);
	bool InBleedout(Actor* actor);
	bool AllowStagger(Actor* giant, Actor* tiny);
	bool IsHuman(Actor* actor);
	bool IsInsect(Actor* actor);
	bool IsFemale(Actor* actor);
	bool IsDragon(Actor* actor);
	bool IsLiving(Actor* actor);
	bool IsMoving(Actor* giant);
	bool IsHeadtracking(Actor* giant);
	// Gts Bools end

	// GTS State Controllers
	void SetBeingHeld(Actor* tiny, bool decide);
	void SetBetweenBreasts(Actor* actor, bool decide);
	void SetBeingEaten(Actor* tiny, bool decide);
	void SetBeingGrinded(Actor* tiny, bool decide);
	void SetCameraOverride(Actor* actor, bool decide);
	void ShutUp(Actor* actor);

	// GTS State Controllers end
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
	void PushActorAway(TESObjectREFR* source, Actor* receiver, NiPoint3 direction, float force);
	void KnockAreaEffect(TESObjectREFR* source, float afMagnitude, float afRadius);
	void ApplyHavokImpulse_Manual(Actor* target, float afX, float afY, float afZ, float afMagnitude);
	void ApplyHavokImpulse(TESObjectREFR* target, float afX, float afY, float afZ, float afMagnitude);

	void CompleteDragonQuest();

	float get_distance_to_actor(Actor* receiver, Actor* target);
	float GetHighHeelsBonusDamage(Actor* actor);

	void ApplyShake(Actor* caster, float modifier);
	void ApplyShakeAtNode(Actor* caster, float modifier, std::string_view node);
	void ApplyShakeAtNode(Actor* caster, float modifier, std::string_view node, float radius);
	void ApplyShakeAtPoint(Actor* caster, float modifier, const NiPoint3& coords, float radius);
	void EnableFreeCamera();

	bool AllowDevourment();
	bool AllowFeetTracking();
	bool LessGore();

	bool IsTeammate(Actor* actor);

	void TrackFeet(Actor* giant, float number, bool enable);
	void CallDevourment(Actor* giant, Actor* tiny);
	void CallGainWeight(Actor* giant, float value);
	void CallVampire();
	void CallHelpMessage();
	void AddCalamityPerk();
	void AddPerkPoints(float level);

	void AddStolenAttributes(Actor* giant, float value);
	float GetStolenAttributes_Values(Actor* giant, ActorValue type);
	float GetStolenAttributes();
	void DistributeStolenAttributes(Actor* giant, float value);

	float GetRandomBoost();
	float GetButtCrushCost(Actor* actor);
	float GetAnimationSlowdown(Actor* giant);

	void DoFootstepSound(Actor* giant, float modifier, FootEvent kind, std::string_view node);
	void DoDustExplosion(Actor* giant, float modifier, FootEvent kind, std::string_view node);
	void SpawnParticle(Actor* actor, float lifetime, const char* modelName, const NiMatrix3& rotation, const NiPoint3& position, float scale, std::uint32_t flags, NiAVObject* target);
	void SpawnDustParticle(Actor* giant, Actor* tiny, std::string_view node, float size);
	void SpawnDustExplosion(Actor* giant, Actor* tiny, std::string_view node, float size);
	void StaggerOr(Actor* giant, Actor* tiny, float power, float afX, float afY, float afZ, float afMagnitude);
	void DoDamageEffect(Actor* giant, float damage, float radius, int random, float bonedamage, FootEvent kind, float crushmult, DamageSource Cause);

	void PushTowards(Actor* giantref, Actor* tinyref, NiAVObject* bone, float power, bool sizecheck);
	void TinyCalamityExplosion(Actor* giant, float radius);
	void ShrinkOuburst_Shrink(Actor* giant, Actor* tiny, float shrink, float gigantism);
	void ShrinkOutburstExplosion(Actor* giant, bool WasHit);
	
	bool HasSMT(Actor* giant);
	void TiredSound(Actor* player, std::string_view message);

	hkaRagdollInstance* GetRagdoll(Actor* actor);

	void ManageRagdoll(Actor* tinyref, float deltaLength, NiPoint3 deltaLocation, NiPoint3 targetLocation);
	void StaggerActor(Actor* receiver);

	float GetMovementModifier(Actor* giant);
	float GetGtsSkillLevel();
	float GetXpBonus();

	void AddSMTDuration(Actor* actor, float duration);
	void AddSMTPenalty(Actor* actor, float penalty);

	void PrintDeathSource(Actor* giant, Actor* tiny, DamageSource cause);
	void PrintSuffocate(Actor* pred, Actor* prey);
	void ShrinkUntil(Actor* giant, Actor* tiny, float expected);
	void DisableCollisions(Actor* actor, TESObjectREFR* otherActor);
	void EnableCollisions(Actor* actor);

 
	void SpringGrow(Actor* actor, float amt, float halfLife, std::string_view naming);
	void SpringGrow_Free(Actor* actor, float amt, float halfLife, std::string_view naming);
	void SpringShrink(Actor* actor, float amt, float halfLife, std::string_view naming);
}
