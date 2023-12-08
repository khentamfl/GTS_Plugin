#include "magic/effects/smallmassivethreat.hpp"
#include "managers/damage/AccurateDamage.hpp"
#include "managers/damage/SizeHitEffects.hpp"
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

	void PushCheck(Actor* giant, Actor* tiny, float force) {
		auto& sizemanager = SizeManager::GetSingleton();
		auto& accuratedamage = AccurateDamage::GetSingleton();
		auto model = tiny->GetCurrent3D();

		if (model) {
			bool isdamaging = sizemanager.IsDamaging(tiny);
			if (!isdamaging && (force >= 0.12 || IsMoving(giant))) {
				StaggerOr(giant, tiny, force, 0, 0, 0, 0);
				sizemanager.GetDamageData(tiny).lastDamageTime = Time::WorldTimeElapsed();
			}
		}
	}

	void ModVulnerability(Actor* giant, Actor* tiny, float damage) {
		if (!Runtime::HasPerkTeam(giant, "GrowingPressure")) {
			return;
		}
		auto& sizemanager = SizeManager::GetSingleton();
		sizemanager.ModSizeVulnerability(tiny, damage * 0.0015);
	}

	void SMTCrushCheck(Actor* Caster, Actor* Target) {
		auto profiler = Profilers::Profile("AccurateDamage: SMTCrushCheck");
		if (Caster == Target) {
			return;
		}
		auto& persistent = Persistent::GetSingleton();
		if (persistent.GetData(Caster)) {
			if (persistent.GetData(Caster)->smt_run_speed >= 1.0) {
				float caster_scale = get_visual_scale(Caster);
				float target_scale = get_visual_scale(Target);
				float Multiplier = (caster_scale/target_scale);
				float CasterHp = Caster->AsActorValueOwner()->GetActorValue(ActorValue::kHealth);
				float TargetHp = Target->AsActorValueOwner()->GetActorValue(ActorValue::kHealth);
				if (CasterHp >= (TargetHp / Multiplier) && !CrushManager::AlreadyCrushed(Target)) {
					CrushManager::Crush(Caster, Target);
					CrushBonuses(Caster, Target);

					Runtime::PlaySound("GtsCrushSound", Caster, 1.0, 1.0);

					shake_camera(Caster, 0.75 * caster_scale, 0.45);
					Cprint("{} was instantly turned into mush by the body of {}", Target->GetDisplayFullName(), Caster->GetDisplayFullName());
					if (Runtime::HasPerk(Caster, "NoSpeedLoss")) {
						AttributeManager::GetSingleton().OverrideSMTBonus(0.65); // Reduce speed after crush
					} else if (!Runtime::HasPerk(Caster, "NoSpeedLoss")) {
						AttributeManager::GetSingleton().OverrideSMTBonus(0.35); // Reduce more speed after crush
					}
				} else if (CasterHp < (TargetHp / Multiplier) && !CrushManager::AlreadyCrushed(Target)) {
					PushForward(Caster, Target, 100);
					AddSMTDuration(Caster, 2.5);
					StaggerActor(Caster);
					Caster->ApplyCurrent(0.5 * target_scale, 0.5 * target_scale);
					Target->AsActorValueOwner()->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, ActorValue::kHealth, -CasterHp * 0.75);
					Caster->AsActorValueOwner()->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage,ActorValue::kHealth, -CasterHp * 0.25);
					shake_camera(Caster, 4.35, 0.5);
					Runtime::PlaySound("lJumpLand", Caster, 1.0, 1.0);

					std::string text_a = Target->GetDisplayFullName();
					std::string text_b = " is too tough to be crushed";
					std::string Message = text_a + text_b;
					DebugNotification(Message.c_str(), 0, true);

					AttributeManager::GetSingleton().OverrideSMTBonus(0.75); // Less speed loss after force crush
				}
			}
		}
	}

	void SizeModifications(Actor* giant, Actor* tiny, float HighHeels) {
		auto profiler = Profilers::Profile("AccurateDamage: SizeModifications");
		if (tiny == giant) {
			return;
		}
		float giantscale = get_visual_scale(giant);
		float tinyscale = get_visual_scale(tiny) * GetScaleAdjustment(tiny);
		float size_difference = giantscale/tinyscale;
		float Gigantism = 1.0 / (1.0 + SizeManager::GetSingleton().GetEnchantmentBonus(giant)/200);
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
				float HpRegen = GetMaxAV(giant, ActorValue::kHealth) * 0.005 * size_difference;
				giant->AsActorValueOwner()->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, ActorValue::kHealth, (HpRegen * TimeScale()) * size_difference);
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
}


namespace Gts {

	AccurateDamage& AccurateDamage::GetSingleton() noexcept {
		static AccurateDamage instance;
		return instance;
	}

	std::string AccurateDamage::DebugName() {
		return "AccurateDamage";
	}

	void AccurateDamage::DoAccurateCollisionLeft(Actor* actor, float damage, float radius, int random, float bbmult, float crushmult, DamageSource Cause) { // Called from GtsManager.cpp, checks if someone is close enough, then calls DoSizeDamage()
		auto profiler = Profilers::Profile("AccurateDamageLeft: DoAccurateCollisionLeft");
		auto& accuratedamage = AccurateDamage::GetSingleton();
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
								float aveForce = std::clamp(force, 0.00f, 0.70f);///nodeCollisions;
								accuratedamage.ApplySizeEffect(actor, otherActor, aveForce * damage, random, bbmult, crushmult, Cause);
							}
						}
					}
				}
			}
		}
	}

	void AccurateDamage::DoAccurateCollisionRight(Actor* actor, float damage, float radius, int random, float bbmult, float crushmult, DamageSource Cause) { // Called from GtsManager.cpp, checks if someone is close enough, then calls DoSizeDamage()
		auto profiler = Profilers::Profile("AccurateDamageRight: DoAccurateCollisionRight");
		auto& accuratedamage = AccurateDamage::GetSingleton();
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
								float aveForce = std::clamp(force, 0.00f, 0.70f);///nodeCollisions;
								accuratedamage.ApplySizeEffect(actor, otherActor, aveForce * damage, random, bbmult, crushmult, Cause);
							}
						}
					}
				}
			}
		}
	}

	void AccurateDamage::ApplySizeEffect(Actor* giant, Actor* tiny, float force, int random, float bbmult, float crushmult, DamageSource Cause) {
		auto profiler = Profilers::Profile("AccurateDamage: ApplySizeEffect");
		auto& accuratedamage = AccurateDamage::GetSingleton();

		float movementFactor = 1.0;
		if (giant->AsActorState()->IsSprinting()) {
			movementFactor *= 1.5;
		}

		PushCheck(giant, tiny, force);

		accuratedamage.DoSizeDamage(giant, tiny, movementFactor, force, random, bbmult, true, crushmult, Cause);
	}

	void AccurateDamage::DoSizeDamage(Actor* giant, Actor* tiny, float totaldamage, float mult, int random, float bbmult, bool DoDamage, float crushmult, DamageSource Cause) { // Applies damage and crushing
		auto profiler = Profilers::Profile("AccurateDamage: DoSizeDamage");
		if (!giant) {
			return;
		}
		if (!tiny) {
			return;
		}
		if (giant == tiny) {
			return;
		}
		if (!CanDoDamage(giant, tiny) && !IsBetweenBreasts(giant)) {
			return;
		}
		auto& sizemanager = SizeManager::GetSingleton();
		auto& crushmanager = CrushManager::GetSingleton();
		float giantsize = get_visual_scale(giant);
		float tinysize = get_visual_scale(tiny) * GetScaleAdjustment(tiny);

		float damagebonus = Persistent::GetSingleton().size_related_damage_mult;

		float highheels = (1.0 + HighHeelManager::GetBaseHHOffset(giant).Length()/200);
		float multiplier = (giantsize/tinysize) * highheels;
		if (HasSMT(giant)) {
			multiplier += 7.8;
			damagebonus *= 0.25; // A lot less damage to compensate it
		}
		if (multiplier < 1.4) {
			return; // Do not do damage is Size Difference is < than x1.4
		}
		float additionaldamage = 1.0 + sizemanager.GetSizeVulnerability(tiny); // Get size damage debuff from enemy
		float normaldamage = std::clamp(sizemanager.GetSizeAttribute(giant, 0) * 0.30, 0.30, 999999.0);
		float highheelsdamage = 1.0 + (GetHighHeelsBonusDamage(giant) * 5);
		float sprintdamage = 1.0; // default Sprint damage of 1.0
		float falldamage = 1.0; // default Fall damage of 1.0
		float weightdamage = 1.0 + (giant->GetWeight()*0.01);

		SizeModifications(giant, tiny, highheels);
		SMTCrushCheck(giant, tiny);

		if (giant->AsActorState()->IsSprinting()) {
			sprintdamage = 1.5 * sizemanager.GetSizeAttribute(giant, 1);
		}

		float result = ((0.125 * multiplier) * totaldamage) * (normaldamage * sprintdamage * falldamage) * (highheelsdamage * weightdamage * mult) * additionaldamage;
		if (giant->IsSneaking()) {
			result *= 0.33;
		}

		SizeHitEffects::GetSingleton().BreakBones(giant, tiny, result * bbmult, random);
		// ^ Chance to break bonues and inflict additional damage, as well as making target more vulerable to size damage


		StartCombat(giant, tiny, false);


		result *= damagebonus;

		if ((Cause == DamageSource::CrushedLeft || Cause == DamageSource::CrushedRight) && Runtime::HasPerkTeam(giant, "hhBonus")) {
			result *= 1.15; // 15% bonus damage if we have High Heels perk
		}

		float experience = std::clamp(result/500, 0.0f, 0.05f);
		if (!tiny->IsDead()) {
			AdjustGtsSkill(experience, giant);
		}

		if (tiny->formID == 0x14 || SizeManager::GetSingleton().BalancedMode() == 2.0 && GetAV(tiny, ActorValue::kStamina) > 2.0) {
			DamageAV(tiny, ActorValue::kStamina, result * 2.0);
			result -= GetAV(tiny, ActorValue::kStamina); // Reduce damage by stamina amount
			if (result < 0) {
				result = 0; // just to be safe
			}
			if (result < GetAV(tiny, ActorValue::kStamina)) {
				return; // Fully protect against size-related damage
			}
		}
		if (DoDamage) {
			ModVulnerability(giant, tiny, result);
			InflictSizeDamage(giant, tiny, result);
		}

		if (GetAV(tiny, ActorValue::kHealth) <= 0 || tiny->IsDead()) {
			ReportCrime(giant, tiny, 1000, true);
			if (multiplier >= 8.0 * crushmult) {
				if (CrushManager::CanCrush(giant, tiny)) {
					crushmanager.Crush(giant, tiny);
					if (!tiny->IsDead()) {
						if (IsGiant(tiny)) {
							AdvanceQuestProgression(giant, 7, 1);
						} else {
							AdvanceQuestProgression(giant, 3, 1);
						}
					}
					
					SetReanimatedState(tiny);
					
					KillActor(giant, tiny);
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
				}
			}
		}
	}
}
