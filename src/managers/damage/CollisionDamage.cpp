#include "magic/effects/smallmassivethreat.hpp"
#include "managers/damage/CollisionDamage.hpp"
#include "managers/damage/SizeHitEffects.hpp"
#include "managers/damage/TinyCalamity.hpp"
#include "managers/RipClothManager.hpp"
#include "managers/ai/aifunctions.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/CrushManager.hpp"
#include "managers/InputManager.hpp"
#include "magic/effects/common.hpp"
#include "managers/GtsManager.hpp"
#include "managers/Attributes.hpp"
#include "managers/hitmanager.hpp"
#include "managers/highheel.hpp"
#include "utils/DeathReport.hpp"
#include "utils/actorUtils.hpp"
#include "data/persistent.hpp"
#include "data/transient.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "UI/DebugAPI.hpp"
#include "data/time.hpp"
#include "profiler.hpp"
#include "Config.hpp"
#include "timer.hpp"
#include "node.hpp"
#include <vector>
#include <string>


using namespace Gts;
using namespace RE;
using namespace SKSE;
using namespace std;

namespace {
	const std::string_view leftFootLookup = "NPC L Foot [Lft ]";
	const std::string_view rightFootLookup = "NPC R Foot [Rft ]";
	const std::string_view leftCalfLookup = "NPC L Calf [LClf]";
	const std::string_view rightCalfLookup = "NPC R Calf [RClf]";
	const std::string_view leftToeLookup = "NPC L Toe0 [LToe]";
	const std::string_view rightToeLookup = "NPC R Toe0 [RToe]";
	const std::string_view bodyLookup = "NPC Spine1 [Spn1]";

	const float LAUNCH_DAMAGE = 1.2f;
	const float LAUNCH_KNOCKBACK = 0.02f;
	const float UNDERFOOT_POWER = 0.60;

	bool CanDoDamage(Actor* giant, Actor* tiny) {
		if (IsBeingHeld(tiny)) {
			return false;
		}
		bool NPC = Persistent::GetSingleton().NPCEffectImmunity;
		bool PC = Persistent::GetSingleton().PCEffectImmunity;
		if (NPC && giant->formID == 0x14 && (IsTeammate(tiny))) {
			return false; // Protect NPC's against player size-related effects
		}
		if (NPC && (IsTeammate(giant)) && (IsTeammate(tiny))) {
			return false; // Disallow NPC's to damage each-other if they're following Player
		}
		if (PC && (IsTeammate(giant)) && tiny->formID == 0x14) {
			return false; // Protect Player against friendly NPC's damage
		}
		return true;
	}

	void ModVulnerability(Actor* giant, Actor* tiny, float damage) {
		if (!Runtime::HasPerkTeam(giant, "GrowingPressure")) {
			return;
		}
		auto& sizemanager = SizeManager::GetSingleton();
		sizemanager.ModSizeVulnerability(tiny, damage * 0.0015);
	}

	void SizeModifications(Actor* giant, Actor* tiny, float HighHeels) {
		auto profiler = Profilers::Profile("CollisionDamage: SizeModifications");
		if (tiny == giant) {
			return;
		}
		float giantscale = get_visual_scale(giant);
		float tinyscale = get_visual_scale(tiny) * GetScaleAdjustment(tiny);
		float size_difference = giantscale/tinyscale;
		float BonusShrink = (IsJumping(giant) * 3.0) + 1.0;

		if (Runtime::HasPerk(giant, "ExtraGrowth") && giant != tiny && HasGrowthSpurt(giant)) {
			if (get_target_scale(tiny) > 0.12) {
				ShrinkActor(tiny, 0.0014 * BonusShrink, 0.0);
				Grow(giant, 0.0, 0.0004 * BonusShrink);
			} else {
				set_target_scale(tiny, 0.12);
			}
			// ^ Augmentation for Growth Spurt: Steal size of enemies.
		}

		if (HasSMT(giant) && giant != tiny) {
			size_difference += 4.0;

			if (Runtime::HasPerk(giant, "SmallMassiveThreatSizeSteal")) {
				float HpRegen = GetMaxAV(giant, ActorValue::kHealth) * 0.0001 * (size_difference - 4.0);
				giant->AsActorValueOwner()->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, ActorValue::kHealth, (HpRegen * TimeScale()) * size_difference);
				//log::info("Hp regen: {}, Regen * Time * Difference: {}, maxHP: {}", HpRegen, (HpRegen * TimeScale()) * size_difference, GetMaxAV(giant, ActorValue::kHealth));
				if (get_target_scale(tiny) > 0.12) {
					ShrinkActor(tiny, 0.0015 * BonusShrink, 0.0);
					Grow(giant, 0.00045 * tinyscale * BonusShrink, 0.0);
				} else {
					set_target_scale(tiny, 0.12);
				}
				// ^ Tiny calamity augmentation, deals pretty much the same as the one above, but a bit stronger
			}
		}
	}

	float HighHeels_PerkDamage(Actor* giant, DamageSource Cause) {
		float value = 1.0;
		bool perk = Runtime::HasPerkTeam(giant, "hhBonus");
		bool matches = (Cause == DamageSource::CrushedLeft || Cause == DamageSource::CrushedRight);
		if (perk && matches) {
			value += 0.15; // 15% bonus damage if we have High Heels perk
		}
		return value;
	}
}


namespace Gts {

	CollisionDamage& CollisionDamage::GetSingleton() noexcept {
		static CollisionDamage instance;
		return instance;
	}

	std::string CollisionDamage::DebugName() {
		return "CollisionDamage";
	}

	void CollisionDamage::DoFootCollision_Left(Actor* actor, float damage, float radius, int random, float bbmult, float crush_threshold, DamageSource Cause) { // Called from GtsManager.cpp, checks if someone is close enough, then calls DoSizeDamage()
		auto profiler = Profilers::Profile("CollisionDamageLeft: DoFootCollision_Left");
		auto& CollisionDamage = CollisionDamage::GetSingleton();
		if (!actor) {
			return;
		}

		float giantScale = get_visual_scale(actor);
		const float BASE_CHECK_DISTANCE = 90.0;
		const float BASE_DISTANCE = 6.0;
		float SCALE_RATIO = 1.15;
		if (HasSMT(actor)) {
			giantScale += 0.20;
			SCALE_RATIO = 0.7;
		}

		// Get world HH offset
		NiPoint3 hhOffset = HighHeelManager::GetHHOffset(actor);
		NiPoint3 hhOffsetbase = HighHeelManager::GetBaseHHOffset(actor);

		auto leftFoot = find_node(actor, leftFootLookup);
		auto leftCalf = find_node(actor, leftCalfLookup);
		auto leftToe = find_node(actor, leftToeLookup);
		if (!leftFoot) {
			return;
		}
		if (!leftCalf) {
			return;
		}
		if (!leftToe) {
			return;
		}
		NiMatrix3 leftRotMat;
		{
			NiAVObject* foot = leftFoot;
			NiAVObject* calf = leftCalf;
			NiAVObject* toe = leftToe;
			NiTransform inverseFoot = foot->world.Invert();
			NiPoint3 forward = inverseFoot*toe->world.translate;
			forward = forward / forward.Length();

			NiPoint3 up = inverseFoot*calf->world.translate;
			up = up / up.Length();

			NiPoint3 right = forward.UnitCross(up);
			forward = up.UnitCross(right); // Reorthonalize

			leftRotMat = NiMatrix3(right, forward, up);
		}

		float maxFootDistance = BASE_DISTANCE * radius * giantScale;
		float hh = hhOffsetbase[2];
		// Make a list of points to check
		std::vector<NiPoint3> points = {
			NiPoint3(0.0, hh*0.08, -0.25 +(-hh * 0.25)), // The standard at the foot position
			NiPoint3(1.6, 7.7 + (hh/70), -0.75 + (-hh * 1.15)), // Offset it forward
			NiPoint3(0.0, (hh/50), -0.25 + (-hh * 1.15)), // Offset for HH
		};
		std::tuple<NiAVObject*, NiMatrix3> left(leftFoot, leftRotMat);

		for (const auto& [foot, rotMat]: {left}) {
			std::vector<NiPoint3> footPoints = {};
			for (NiPoint3 point: points) {
				footPoints.push_back(foot->world*(rotMat*point));
			}
			if (IsDebugEnabled() && (actor->formID == 0x14 || IsTeammate(actor) || EffectsForEveryone(actor))) {
				for (auto point: footPoints) {
					DebugAPI::DrawSphere(glm::vec3(point.x, point.y, point.z), maxFootDistance);
				}
			}

			NiPoint3 giantLocation = actor->GetPosition();
			for (auto otherActor: find_actors()) {
				if (otherActor != actor) {
					float tinyScale = get_visual_scale(otherActor);
					if (giantScale / tinyScale > SCALE_RATIO) {
						NiPoint3 actorLocation = otherActor->GetPosition();

						if ((actorLocation-giantLocation).Length() < BASE_CHECK_DISTANCE*giantScale) {
							// Check the tiny's nodes against the giant's foot points
							int nodeCollisions = 0;
							float force = 0.0;

							auto model = otherActor->GetCurrent3D();

							if (model) {
								for (auto point: footPoints) {
									VisitNodes(model, [&nodeCollisions, &force, point, maxFootDistance](NiAVObject& a_obj) {
										float distance = (point - a_obj.world.translate).Length();
										if (distance < maxFootDistance) {
											nodeCollisions += 1;
											force = 1.0 - distance / maxFootDistance;//force += 1.0 - distance / maxFootDistance;
										}
										return true;
									});
								}
							}
							if (nodeCollisions > 0) {
								Utils_PushCheck(giant, tiny, force); // pass original un-altered force
								float aveForce = std::clamp(force, 0.15f, 0.70f);///nodeCollisions;
								CollisionDamage.ApplySizeEffect(actor, otherActor, aveForce * damage, random, bbmult, crush_threshold, Cause);
							}
						}
					}
				}
			}
		}
	}

	void CollisionDamage::DoFootCollision_Right(Actor* actor, float damage, float radius, int random, float bbmult, float crush_threshold, DamageSource Cause) { // Called from GtsManager.cpp, checks if someone is close enough, then calls DoSizeDamage()
		auto profiler = Profilers::Profile("CollisionDamageRight: DoFootCollision_Right");
		auto& CollisionDamage = CollisionDamage::GetSingleton();
		if (!actor) {
			return;
		}

		float giantScale = get_visual_scale(actor);
		const float BASE_CHECK_DISTANCE = 90.0;
		const float BASE_DISTANCE = 6.0;
		float SCALE_RATIO = 1.15;
		if (HasSMT(actor)) {
			giantScale += 0.20;
			SCALE_RATIO = 0.7;
		}

		// Get world HH offset
		NiPoint3 hhOffset = HighHeelManager::GetHHOffset(actor);
		NiPoint3 hhOffsetbase = HighHeelManager::GetBaseHHOffset(actor);

		auto rightFoot = find_node(actor, rightFootLookup);
		auto rightCalf = find_node(actor, rightCalfLookup);
		auto rightToe = find_node(actor, rightToeLookup);


		if (!rightFoot) {
			return;
		}
		if (!rightCalf) {
			return;
		}
		if (!rightToe) {
			return;
		}
		NiMatrix3 rightRotMat;
		{
			NiAVObject* foot = rightFoot;
			NiAVObject* calf = rightCalf;
			NiAVObject* toe = rightToe;

			NiTransform inverseFoot = foot->world.Invert();
			NiPoint3 forward = inverseFoot*toe->world.translate;
			forward = forward / forward.Length();

			NiPoint3 up = inverseFoot*calf->world.translate;
			up = up / up.Length();

			NiPoint3 right = up.UnitCross(forward);
			forward = right.UnitCross(up); // Reorthonalize

			rightRotMat = NiMatrix3(right, forward, up);
		}

		float maxFootDistance = BASE_DISTANCE * radius * giantScale;
		float hh = hhOffsetbase[2];
		// Make a list of points to check
		std::vector<NiPoint3> points = {
			NiPoint3(0.0, hh*0.08, -0.25 +(-hh * 0.25)), // The standard at the foot position
			NiPoint3(-1.6, 7.7 + (hh/70), -0.75 + (-hh * 1.15)), // Offset it forward
			NiPoint3(0.0, (hh/50), -0.25 + (-hh * 1.15)), // Offset for HH
		};
		std::tuple<NiAVObject*, NiMatrix3> right(rightFoot, rightRotMat);

		for (const auto& [foot, rotMat]: {right}) {
			std::vector<NiPoint3> footPoints = {};
			for (NiPoint3 point: points) {
				footPoints.push_back(foot->world*(rotMat*point));
			}
			if (IsDebugEnabled() && (actor->formID == 0x14 || IsTeammate(actor) || EffectsForEveryone(actor))) {
				for (auto point: footPoints) {
					DebugAPI::DrawSphere(glm::vec3(point.x, point.y, point.z), maxFootDistance);
				}
			}

			NiPoint3 giantLocation = actor->GetPosition();
			for (auto otherActor: find_actors()) {
				if (otherActor != actor) {
					float tinyScale = get_visual_scale(otherActor);
					if (giantScale / tinyScale > SCALE_RATIO) {
						NiPoint3 actorLocation = otherActor->GetPosition();

						if ((actorLocation-giantLocation).Length() < BASE_CHECK_DISTANCE*giantScale) {
							// Check the tiny's nodes against the giant's foot points
							int nodeCollisions = 0;
							float force = 0.0;

							auto model = otherActor->GetCurrent3D();

							if (model) {
								for (auto point: footPoints) {
									VisitNodes(model, [&nodeCollisions, &force, point, maxFootDistance](NiAVObject& a_obj) {
										float distance = (point - a_obj.world.translate).Length();
										if (distance < maxFootDistance) {
											nodeCollisions += 1;
											force = 1.0 - distance / maxFootDistance;//force += 1.0 - distance / maxFootDistance;
										}
										return true;
									});
								}
							}
							if (nodeCollisions > 0) {
								Utils_PushCheck(giant, tiny, force); // pass original un-altered force
								float aveForce = std::clamp(force, 0.15f, 0.70f);///nodeCollisions;
								CollisionDamage.ApplySizeEffect(actor, otherActor, aveForce * damage, random, bbmult, crush_threshold, Cause);
							}
						}
					}
				}
			}
		}
	}

	void CollisionDamage::ApplySizeEffect(Actor* giant, Actor* tiny, float force, int random, float bbmult, float crush_threshold, DamageSource Cause) {
		auto profiler = Profilers::Profile("CollisionDamage: ApplySizeEffect");
		auto& CollisionDamage = CollisionDamage::GetSingleton();

		if (giant->AsActorState()->IsSprinting()) {
			force *= 1.5;
			// Force acts as a damage here. 
			// DoDamageEffect() (ActorUtils.cpp) function sends 45.0 by default and then further multiplies it based on animation damage setting
			//
			// So, force = (force * 45 * modifier from animation)             (True only when actors performs animation)
			// Else force is normal and damage is very low (in case of just standing still)
		}

		CollisionDamage.DoSizeDamage(giant, tiny, force, bbmult, crush_threshold, random, Cause);
	}

	void CollisionDamage::DoSizeDamage(Actor* giant, Actor* tiny, float damage, float bbmult, float crush_threshold, int random, DamageSource Cause) { // Applies damage and crushing
		auto profiler = Profilers::Profile("CollisionDamage: DoSizeDamage");
		if (!giant) {
			return;
		}
		if (!tiny) {
			return;
		}
		if (giant == tiny) {
			return;
		}
		if (!CanDoDamage(giant, tiny) || IsBetweenBreasts(giant)) { // disallow if 
			log::info("Can do damage Check isn't passed");
			return;
		}

		auto& sizemanager = SizeManager::GetSingleton();

		float highheels = (1.0 + HighHeelManager::GetBaseHHOffset(giant).Length()/200);
		float size_difference = GetSizeDifference(giant, tiny) * highheels;

		if (size_difference < 1.4 || DisallowSizeDamage(giant, tiny)) {
			log::info("Size differnce is not passed: {}, diallow: {}", size_difference, DisallowSizeDamage(giant, tiny));
			return; // Do not do damage is Size Difference is < than x1.4 or we want to protect a tiny 
			// when under the effect of non-hostile protection
		}

		float damagebonus = HighHeels_PerkDamage(giant, Cause); // 15% bonus HH damage if we have perk
		if (HasSMT(giant)) {
			damagebonus *= 0.25; // A lot less damage to compensate it
		}

		float additionaldamage = 1.0 + sizemanager.GetSizeVulnerability(tiny); // Get size damage debuff from enemy
		float normaldamage = std::clamp(sizemanager.GetSizeAttribute(giant, 0) * 0.30, 0.30, 999999.0);
		float highheelsdamage = 1.0 + (GetHighHeelsBonusDamage(giant) * 5);
		float sprintdamage = 1.0; // default Sprint damage of 1.0
		// fall damage is unused since it is always = 1.0
		float weightdamage = 1.0 + (giant->GetWeight()*0.01);

		SizeModifications(giant, tiny, highheels);
		TinyCalamity_CrushCheck(giant, tiny);

		if (giant->AsActorState()->IsSprinting()) {
			sprintdamage = 1.5 * sizemanager.GetSizeAttribute(giant, 1);
		}

		float damage_result = (0.125 * size_difference) * (normaldamage * sprintdamage) * (highheelsdamage * weightdamage * damage) * additionaldamage;
		if (giant->IsSneaking()) {
			damage_result *= 0.66;
		}

		SizeHitEffects::GetSingleton().BreakBones(giant, tiny, damage_result * bbmult, random);
		// ^ Chance to break bonues and inflict additional damage, as well as making target more vulerable to size damage

		damage_result *= damagebonus;
		log::info("Damage Result of {} is {}", giant->GetDisplayFullName(), damage_result);

		if (!tiny->IsDead()) {
			float experience = std::clamp(damage_result/500, 0.0f, 0.05f);
			ModSizeExperience(giant, experience);
		}

		if (tiny->formID == 0x14 || SizeManager::GetSingleton().BalancedMode() == 2.0 && GetAV(tiny, ActorValue::kStamina) > 2.0) {
			DamageAV(tiny, ActorValue::kStamina, damage_result * 2.0);
			damage_result -= GetAV(tiny, ActorValue::kStamina); // Reduce damage by stamina amount
			if (damage_result < 0) {
				damage_result = 0; // just to be safe and to not restore attributes
			}
			if (damage_result < GetAV(tiny, ActorValue::kStamina)) {
				return; // Fully protect against size-related damage
			}
		}
		
		ModVulnerability(giant, tiny, damage_result);
		InflictSizeDamage(giant, tiny, damage_result);

		CollisionDamage::CrushCheck(giant, tiny, size_difference, crush_threshold, Cause);
	}

	void CollisionDamage::CrushCheck(Actor* giant, Actor* tiny, float size_difference, float crush_threshold, DamageSource Cause) {
		bool CanBeCrushed = (
			GetAV(tiny, ActorValue::kHealth) <= 1.0 ||
			tiny->IsDead()
		);
		
		if (CanBeCrushed) {
			if (size_difference > 8.0 * crush_threshold && CrushManager::CanCrush(giant, tiny)) {
				ModSizeExperience_Crush(giant, tiny, true);

				if (!tiny->IsDead()) {
					if (IsGiant(tiny)) {
						AdvanceQuestProgression(giant, tiny, 7, 1, false);
					} else {
						AdvanceQuestProgression(giant, tiny, 3, 1, false);
					}
				}

				SetReanimatedState(tiny);

				CrushBonuses(giant, tiny);
				PrintDeathSource(giant, tiny, Cause);
				if (!LessGore()) {
					auto node = find_node(giant, GetDeathNodeName(Cause));
					if (node) {
						if (IsMechanical(tiny)) {
							return;
						} else {
							Runtime::PlaySoundAtNode("GtsCrushSound", giant, 1.0, 1.0, node);
						}
					} else {
						Runtime::PlaySound("GtsCrushSound", giant, 1.0, 1.0);
					}
				}

				CrushManager::GetSingleton().Crush(giant, tiny);
			}
		}
	}
}
