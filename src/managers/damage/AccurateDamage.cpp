#include "Config.hpp"
#include "managers/GrowthTremorManager.hpp"
#include "managers/RipClothManager.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/damage/AccurateDamage.hpp"
#include "managers/GtsManager.hpp"
#include "managers/highheel.hpp"
#include "managers/Attributes.hpp"
#include "managers/CrushManager.hpp"
#include "managers/InputManager.hpp"
#include "managers/hitmanager.hpp"
#include "magic/effects/smallmassivethreat.hpp"
#include "magic/effects/common.hpp"
#include "data/persistent.hpp"
#include "data/transient.hpp"
#include "data/runtime.hpp"
#include "data/time.hpp"
#include "scale/scale.hpp"
#include "scale/scalespellmanager.hpp"
#include "utils/actorUtils.hpp"
#include "node.hpp"
#include "timer.hpp"
#include <vector>
#include <string>
#include "UI/DebugAPI.hpp"

using namespace Gts;
using namespace RE;
using namespace SKSE;
using namespace std;

namespace {
	const float LAUNCH_DAMAGE = 0.4f;
	const float LAUNCH_KNOCKBACK = 0.02f;
	const float UNDERFOOT_POWER = 0.60;

	void StaggerOr(Actor* giant, Actor* tiny, float power) {
		if (tiny->IsDead()) {
			return;
		}
		bool hasSMT = Runtime::HasMagicEffect(giant, "SmallMassiveThreat");
		float giantSize = get_visual_scale(giant);
		float tinySize = get_visual_scale(tiny);
		if (hasSMT) {
			giantSize *= 4.0;
		}
		float sizedifference = giantSize/tinySize;
		int ragdollchance = rand() % 30 + 1.0;
		if (sizedifference >= 3.0) {
			PushActorAway(giant, tiny, power); // Always push
			return;
		}
		if (ragdollchance < 30.0/sizedifference && sizedifference >= 1.33 && sizedifference < 3.0) {
			tiny->SetGraphVariableFloat("staggerMagnitude", 100.00f); // Stagger actor
			tiny->NotifyAnimationGraph("staggerStart");
			return;
		} else if (ragdollchance == 31.0) {
			PushActorAway(giant, tiny, power); // Push instead
			return;
		}
	}


	void SMTCrushCheck(Actor* Caster, Actor* Target) {
		if (Caster == Target) {
			return;
		}
		auto& persistent = Persistent::GetSingleton();
		if (persistent.GetData(Caster)) {
			if (persistent.GetData(Caster)->smt_run_speed >= 1.0) {
				float caster_scale = get_target_scale(Caster);
				float target_scale = get_target_scale(Target);
				float Multiplier = (caster_scale/target_scale);
				float CasterHp = Caster->AsActorValueOwner()->GetActorValue(ActorValue::kHealth);
				float TargetHp = Target->AsActorValueOwner()->GetActorValue(ActorValue::kHealth);
				if (CasterHp >= (TargetHp / Multiplier) && !CrushManager::AlreadyCrushed(Target)) {
					CrushManager::Crush(Caster, Target);
					CrushBonuses(Caster, Target, 0);
					shake_camera(Caster, 0.75 * caster_scale, 0.45);
					ConsoleLog::GetSingleton()->Print("%s was instantly turned into mush by the body of %s", Target->GetDisplayFullName(), Caster->GetDisplayFullName());
					if (Runtime::HasPerk(Caster, "NoSpeedLoss")) {
						AttributeManager::GetSingleton().OverrideSMTBonus(0.65); // Reduce speed after crush
					} else if (!Runtime::HasPerk(Caster, "NoSpeedLoss")) {
						AttributeManager::GetSingleton().OverrideSMTBonus(0.35); // Reduce more speed after crush
					}
				} else if (CasterHp < (TargetHp / Multiplier) && !CrushManager::AlreadyCrushed(Target)) {
					PushActorAway(Caster, Target, 0.8);
					PushActorAway(Target, Caster, 0.2);
					Caster->ApplyCurrent(0.5 * target_scale, 0.5 * target_scale); Target->ApplyCurrent(0.5 * caster_scale, 0.5 * caster_scale);  // Else simulate collision
					Target->AsActorValueOwner()->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, ActorValue::kHealth, -CasterHp * 0.75);
					Caster->AsActorValueOwner()->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage,ActorValue::kHealth, -CasterHp * 0.25);
					shake_camera(Caster, 2.35, 0.5);
					Runtime::PlaySound("lJumpLand", Caster, 0.5, 1.0);

					std::string text_a = Target->GetDisplayFullName();
					std::string text_b = " is too tough to be crushed";
					std::string Message = text_a + text_b;
					DebugNotification(Message.c_str(), 0, true);

					AttributeManager::GetSingleton().OverrideSMTBonus(0.75); // Less speed loss after force crush
				}
			}
		}
	}

	void ApplySizeEffect(Actor* giant, Actor* tiny, float force) {
		if (Runtime::GetBool("GtsNPCEffectImmunityToggle") && giant->formID == 0x14 && tiny->IsPlayerTeammate()) {
			return;
		}
		if (Runtime::GetBool("GtsNPCEffectImmunityToggle") && giant->IsPlayerTeammate() && tiny->IsPlayerTeammate()) {
			return;
		}
		if (Runtime::GetBool("GtsPCEffectImmunityToggle") && tiny->formID == 0x14) {
			return;
		}
		auto& sizemanager = SizeManager::GetSingleton();
		auto& accuratedamage = AccurateDamage::GetSingleton();
		auto model = tiny->GetCurrent3D();

		if (model) {
			bool isdamaging = sizemanager.IsDamaging(tiny);
			float movementFactor = 1.0;
			if (giant->AsActorState()->IsSprinting()) {
				movementFactor *= 1.5;
			}
			if (!isdamaging && !giant->AsActorState()->IsSprinting() && !giant->AsActorState()->IsWalking() && !giant->IsRunning()) {
				StaggerOr(giant, tiny, 1 * force);
				sizemanager.GetDamageData(tiny).lastDamageTime = Time::WorldTimeElapsed();
				accuratedamage.DoSizeDamage(giant, tiny, movementFactor, force, true);
			}
			if (!isdamaging && (force >= 0.55 || giant->AsActorState()->IsSprinting() || giant->AsActorState()->IsWalking() || giant->IsRunning() || giant->IsSneaking())) {
				StaggerOr(giant, tiny, 1 * force);
				sizemanager.GetDamageData(tiny).lastDamageTime = Time::WorldTimeElapsed();
			}
			accuratedamage.DoSizeDamage(giant, tiny, movementFactor, force, true);
		}
	}


	void SizeModifications(Actor* giant, Actor* tiny, float HighHeels) {
		float InstaCrushRequirement = 24.0;
		float giantscale = get_visual_scale(giant);
		float tinyscale = get_visual_scale(tiny);
		if (IsDragon(tiny)) {
			tinyscale *= 2.0;
		}
		float size_difference = giantscale/tinyscale;
		float Gigantism = 1.0 / (1.0 + SizeManager::GetSingleton().GetEnchantmentBonus(giant)/200);
		float BonusShrink = (IsJumping(giant) * 3.0) + 1.0;

		if (tiny == giant) {
			return;
		}

		if (Runtime::HasPerk(giant, "LethalSprint") && giant->AsActorState()->IsSprinting()) {
			InstaCrushRequirement = (18.0 / HighHeels) * Gigantism;
		}

		if (Runtime::HasPerk(giant, "ExtraGrowth") && giant != tiny && (Runtime::HasMagicEffect(giant, "explosiveGrowth1") || Runtime::HasMagicEffect(giant, "explosiveGrowth2") || Runtime::HasMagicEffect(giant, "explosiveGrowth3"))) {
			ShrinkActor(tiny, 0.0014 * BonusShrink, 0.0);
			Grow(giant, 0.0, 0.0004 * BonusShrink);
			// ^ Augmentation for Growth Spurt: Steal size of enemies.
		}

		if (Runtime::HasMagicEffect(giant, "SmallMassiveThreat") && giant != tiny) {
			size_difference += 4.0;

			if (Runtime::HasPerk(giant, "SmallMassiveThreatSizeSteal")) {
				float HpRegen = GetMaxAV(giant, ActorValue::kHealth) * 0.005 * size_difference;
				giant->AsActorValueOwner()->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, ActorValue::kHealth, (HpRegen * TimeScale()) * size_difference);
				ShrinkActor(tiny, 0.0015 * BonusShrink, 0.0);
				Grow(giant, 0.00045 * tinyscale * BonusShrink, 0.0);
			}
		}

		if (size_difference >= InstaCrushRequirement && !tiny->IsPlayerTeammate()) {
			CrushManager::Crush(giant, tiny);
			CrushBonuses(giant, tiny, 0);
			KnockAreaEffect(giant, 2, 16 * giantscale);
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

	void AccurateDamage::DoAccurateCollision(Actor* actor) { // Called from GtsManager.cpp, checks if someone is close enough, then calls DoSizeDamage()
		auto& accuratedamage = AccurateDamage::GetSingleton();
		if (!actor) {
			return;
		}
		float actualGiantScale = get_visual_scale(actor);
		float giantScale = get_visual_scale(actor);
		const float BASE_CHECK_DISTANCE = 90;
		const float BASE_DISTANCE = 6.0;
		const float SCALE_RATIO = 1.15;

		// Get world HH offset
		NiPoint3 hhOffset = HighHeelManager::GetHHOffset(actor);
		NiPoint3 hhOffsetbase = HighHeelManager::GetBaseHHOffset(actor);

		const std::string_view leftFootLookup = "NPC L Foot [Lft ]";
		const std::string_view rightFootLookup = "NPC R Foot [Rft ]";
		const std::string_view leftCalfLookup = "NPC L Calf [LClf]";
		const std::string_view rightCalfLookup = "NPC R Calf [RClf]";
		const std::string_view leftToeLookup = "NPC L Toe0 [LToe]";
		const std::string_view rightToeLookup = "NPC R Toe0 [RToe]";

		const std::string_view bodyLookup = "NPC Spine1 [Spn1]";

		auto leftFoot = find_node(actor, leftFootLookup);
		auto rightFoot = find_node(actor, rightFootLookup);

		auto leftCalf = find_node(actor, leftCalfLookup);
		auto rightCalf = find_node(actor, rightCalfLookup);


		auto leftToe = find_node(actor, leftToeLookup);
		auto rightToe = find_node(actor, rightToeLookup);

		auto BodyBone = find_node(actor, bodyLookup);


		if (!leftFoot) {
			return;
		}
		if (!rightFoot) {
			return;
		}
		if (!leftCalf) {
			return;
		}
		if (!rightCalf) {
			return;
		}
		if (!leftToe) {
			return;
		}
		if (!rightToe) {
			return;
		}
		if (!BodyBone) {
			return; // CTD protection attempts
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

		float maxFootDistance = BASE_DISTANCE * giantScale;
		float hh = hhOffsetbase[2];
		// Make a list of points to check
		std::vector<NiPoint3> points = {
			NiPoint3(0.0, hh*0.08, -0.25 +(-hh * 0.25)), // The standard at the foot position
			NiPoint3(-1.6, 7.7 + (hh/70), -0.75 + (-hh * 1.15)), // Offset it forward
			NiPoint3(0.0, (hh/50), -0.25 + (-hh * 1.15)), // Offset for HH
		};
		std::tuple<NiAVObject*, NiMatrix3> left(leftFoot, leftRotMat);
		std::tuple<NiAVObject*, NiMatrix3> right(rightFoot, rightRotMat);

		for (const auto& [foot, rotMat]: {left, right}) {
			std::vector<NiPoint3> footPoints = {};
			for (NiPoint3 point: points) {
				footPoints.push_back(foot->world*(rotMat*point));
			}
			if (Runtime::GetBool("EnableDebugOverlay") && (actor->formID == 0x14 || actor->IsPlayerTeammate() || Runtime::InFaction(actor, "FollowerFaction"))) {
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
											force += 1.0 - distance / maxFootDistance;
										}
										return true;
									});
								}
							}
							if (nodeCollisions > 0) {
								float aveForce = force/50;///nodeCollisions;
								ApplySizeEffect(actor, otherActor, aveForce);
								//break;
							}
						}
					}
				}
			}
		}
	}


	void AccurateDamage::UnderFootEvent(const UnderFoot& evt) { // On underfoot event
		auto giant = evt.giant;
		auto tiny = evt.tiny;
		float force = evt.force;
		if (Runtime::GetBool("GtsNPCEffectImmunityToggle") && giant->formID == 0x14 && tiny->IsPlayerTeammate()) {
			return;
		}
		if (Runtime::GetBool("GtsNPCEffectImmunityToggle") && giant->IsPlayerTeammate() && tiny->IsPlayerTeammate()) {
			return;
		}
		if (Runtime::GetBool("GtsPCEffectImmunityToggle") && tiny->formID == 0x14) {
			return;
		}

		float giantSize = get_visual_scale(giant);
		bool hasSMT = Runtime::HasMagicEffect(giant, "SmallMassiveThreat");
		if (hasSMT) {
			giantSize *= 8.0;
		}
		auto& sizemanager = SizeManager::GetSingleton();
		auto& crushmanager = CrushManager::GetSingleton();
		float tinySize = get_visual_scale(tiny);
		if (IsDragon(tiny)) {
			tinySize *= 2.0;
		}

		float movementFactor = 1.0;

		if (giant->IsSneaking()) {
			movementFactor *= 0.5;
		}
		if (giant->AsActorState()->IsSprinting()) {
			movementFactor *= 1.75;
		}
		if (evt.footEvent == FootEvent::JumpLand) {
			movementFactor *= 3.0;
		}

		float sizeRatio = giantSize/tinySize * movementFactor;
		float knockBack = LAUNCH_KNOCKBACK  * giantSize * movementFactor * force;

		if (force > UNDERFOOT_POWER && sizeRatio >= 2.0) { // If under the foot
			DoSizeDamage(giant, tiny, movementFactor, force * 6, true);

			if (!sizemanager.IsLaunching(tiny)) {
				sizemanager.GetSingleton().GetLaunchData(tiny).lastLaunchTime = Time::WorldTimeElapsed();
				StaggerOr(giant, tiny, knockBack);
			}
		} else if (!sizemanager.IsLaunching(tiny) && force <= UNDERFOOT_POWER) {
			if (Runtime::HasPerkTeam(giant, "LaunchPerk")) {
				if (sizeRatio >= 6.0) {
					// Launch
					sizemanager.GetSingleton().GetLaunchData(tiny).lastLaunchTime = Time::WorldTimeElapsed();
					if (Runtime::HasPerkTeam(giant, "LaunchDamage")) {
						float damage = LAUNCH_DAMAGE * giantSize * movementFactor * force/UNDERFOOT_POWER;
						DamageAV(tiny,ActorValue::kHealth, damage);
					}
					StaggerOr(giant, tiny, knockBack);
					ApplyHavokImpulse(tiny, 0, 0, 50 * movementFactor * giantSize * force, 35 * movementFactor * giantSize * force);
				}
			}
		}
	}

	void AccurateDamage::DoSizeDamage(Actor* giant, Actor* tiny, float totaldamage, float mult, bool DoDamage) { // Applies damage and crushing
		if (!giant) {
			return;
		}
		if (!tiny) {
			return;
		}
		if (giant == tiny) {
			return;
		}
		if (Runtime::GetBool("GtsNPCEffectImmunityToggle") && giant->formID == 0x14 && tiny->IsPlayerTeammate()) {
			return;
		}
		if (Runtime::GetBool("GtsNPCEffectImmunityToggle") && giant->IsPlayerTeammate() && tiny->IsPlayerTeammate()) {
			return;
		}
		if (Runtime::GetBool("GtsPCEffectImmunityToggle") && tiny->formID == 0x14) {
			return;
		}
		//log::info("Doing size damage: {} to {}, Do Damage?: {}", giant->GetDisplayFullName(), tiny->GetDisplayFullName(), DoDamage);
		auto& sizemanager = SizeManager::GetSingleton();
		auto& crushmanager = CrushManager::GetSingleton();
		float giantsize = get_visual_scale(giant);
		float tinysize = get_visual_scale(tiny);
		if (IsDragon(tiny)) {
			tinysize *= 2.0;
		}
		float highheels = (1.0 + HighHeelManager::GetBaseHHOffset(giant).Length()/200);
		float multiplier = giantsize/tinysize * highheels;
		float additionaldamage = 1.0 + sizemanager.GetSizeVulnerability(tiny); // Get size damage debuff from enemy
		float normaldamage = std::clamp(sizemanager.GetSizeAttribute(giant, 0) * 0.25, 0.25, 999999.0);
		float highheelsdamage = sizemanager.GetSizeAttribute(giant, 3);
		float sprintdamage = 1.0; // default Sprint damage of 1.0
		float falldamage = 1.0; // default Fall damage of 1.0
		float weightdamage = giant->GetWeight()/100 + 1.0;

		SizeModifications(giant, tiny, highheels);
		SMTCrushCheck(giant, tiny);

		if (giant->AsActorState()->IsSprinting()) {
			sprintdamage = 1.5 * sizemanager.GetSizeAttribute(giant, 1);
		}
		if (IsJumping(giant)) {
			falldamage = sizemanager.GetSizeAttribute(giant, 2) * 2.0;
		}

		float result = ((0.25 * multiplier) * totaldamage) * (normaldamage * sprintdamage * falldamage) * (highheelsdamage * additionaldamage * weightdamage * mult);
		if (giant->IsSneaking()) {
			result *= 0.33;
		}

		if (Runtime::HasMagicEffect(giant, "SmallMassiveThreat")) {
			multiplier += 7.2;
			result *= 4.0;
		}

		if (multiplier >= 8.0 && (GetAV(tiny, ActorValue::kHealth) <= (result))) {
			if (CrushManager::CanCrush(giant, tiny)) {
				crushmanager.Crush(giant, tiny);
				CrushBonuses(giant, tiny, 0);
			}
		}
		if (SizeManager::GetSingleton().BalancedMode() == 2.0 && GetAV(tiny, ActorValue::kStamina) > 2.0) {
			DamageAV(tiny, ActorValue::kStamina, result * 0.30);
			return; // Stamina protection, emulates Size Damage resistance
		}
		if (!DoDamage) {
			//log::info("Damage is false, returning");
			return;
		}
		DamageAV(tiny, ActorValue::kHealth, result);
	}
}

