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
#include "node.hpp"
#include "timer.hpp"
#include <vector>
#include <string>

using namespace Gts;
using namespace RE;
using namespace SKSE;
using namespace std;

namespace {
	void SizeModifications(Actor* giant, Actor* target, float HighHeels) {
		float InstaCrushRequirement = 24.0;
		float giantscale = get_visual_scale(giant);
		float targetscale = get_visual_scale(target);
		float size_difference = giantscale/targetscale;
		float Gigantism = 1.0 - SizeManager::GetSingleton().GetEnchantmentBonus(giant)/200;
		float BonusShrink = (IsJumping(giant) * 3.0) + 1.0;

			if (CrushManager::AlreadyCrushed(target)) {
				return;
			}

			if (Runtime::HasPerk(giant, "LethalSprint") && giant->IsSprinting()) {
				InstaCrushRequirement = 18.0 * HighHeels * Gigantism;
			}

			if (size_difference >= InstaCrushRequirement && !target->IsPlayerTeammate()) {
				CrushManager::Crush(giant, target);
				CrushToNothing(giant, target);
			}

			if (Runtime::HasPerk(giant, "ExtraGrowth") && giant != target && (Runtime::HasMagicEffect(giant, "explosiveGrowth1") || Runtime::HasMagicEffect(giant, "explosiveGrowth2") || Runtime::HasMagicEffect(giant, "explosiveGrowth3"))) {
				ShrinkActor(giant, 0.0014 * BonusShrink, 0.0);
				Grow(giant, 0.0, 0.0004 * BonusShrink);
				// ^ Augmentation for Growth Spurt: Steal size of enemies.
			}

			if (Runtime::HasMagicEffect(giant, "SmallMassiveThreat") && giant != target) {
				size_difference += 7.2; // Allows to crush same size targets.

			if (Runtime::HasPerk(giant, "SmallMassiveThreatSizeSteal")) {
				float HpRegen = GetMaxAV(giant, ActorValue::kHealth) * 0.005 * size_difference;
				giant->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, ActorValue::kHealth, (HpRegen * TimeScale()) * size_difference);
				ShrinkActor(giant, 0.0015 * BonusShrink, 0.0);
				Grow(giant, 0.00045 * targetscale * BonusShrink, 0.0);
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

	void AccurateDamage::DoAccurateCollision(Actor* actor) { // Called from GtsManager.cpp, checks if someone is close enough, then calls DoSizeDamage()
		if (!SizeManager::GetSingleton().GetPreciseDamage()) {
			return;
		}
			float giantScale = get_visual_scale(actor);
			const float BASE_DISTANCE = 16.0;
			const float BASE_FOOT_DISTANCE = 10.0;
			const float SCALE_RATIO = 2.0;
		for (auto otherActor: find_actors()) {
			if (Runtime::GetBool("GtsNPCEffectImmunityToggle") && actor->formID == 0x14 && otherActor->IsPlayerTeammate()) {
				return;
			} if (Runtime::GetBool("GtsNPCEffectImmunityToggle") && actor->IsPlayerTeammate() && otherActor->IsPlayerTeammate()) {
				return;
			} if (Runtime::GetBool("GtsPCEffectImmunityToggle") && otherActor->formID == 0x14) {
				return;
			}	
			 if (otherActor != actor) {
				float tinyScale = get_visual_scale(otherActor);
				if (giantScale / tinyScale > SCALE_RATIO) {
					NiPoint3 actorLocation = otherActor->GetPosition();
					const std::string_view leftFootLookup = "NPC L Foot [Lft ]";
					const std::string_view rightFootLookup = "NPC R Foot [Rft ]";
					auto leftFoot = find_node(actor, leftFootLookup);
				    auto rightFoot = find_node(actor, rightFootLookup);
					    for (auto foot: {leftFoot, rightFoot}) {
							NiPoint3 footLocatation = foot->world.translate;
							float distance = (footLocatation - actorLocation).Length();
							if (distance < BASE_DISTANCE * giantScale) {
								auto model = otherActor->GetCurrent3D();
								if (Runtime::HasMagicEffect(actor, "SmallMassiveThreat")) {
									giantScale *= 2.0;
								}
							if (model) {
								std::vector<NiAVObject*> bodyParts = {};
								float force = 0.0;
								float footDistance = BASE_DISTANCE*giantScale;
								VisitNodes(model, [footLocatation, footDistance, &bodyParts, &force](NiAVObject& a_obj) {
								float distance = (a_obj.world.translate - footLocatation).Length();
							if (distance < footDistance) {
										bodyParts.push_back(&a_obj);
										force += 1.0 - distance / footDistance;
								}
								return true;
							});
								if (!bodyParts.empty()) {
									auto sizemanager = SizeManager::GetSingleton();
									bool IsDamaging = sizemanager.IsDamaging(otherActor);
									float movementFactor = 1.0;
									if (actor->IsSprinting()) {
										movementFactor *= 1.5;
									}

									float aveForce = force / bodyParts.size();
								if (!IsDamaging && !actor->IsSprinting() && !actor->IsWalking() && !actor->IsRunning()) {
									log::info("Pushing actor away");
									PushActorAway(actor, otherActor, 50 * aveForce);
									sizemanager.GetDamageData(otherActor).lastDamageTime = Time::WorldTimeElapsed();
									DoSizeDamage(actor, otherActor, movementFactor, 1.0 * aveForce); // Apply Damage
									}
								if (force >= 0.20 || actor->IsSprinting() || actor->IsWalking() || actor->IsRunning() || actor->IsSneaking())
									sizemanager.GetDamageData(otherActor).lastDamageTime = Time::WorldTimeElapsed();
									DoSizeDamage(actor, otherActor, movementFactor, 0.60 * aveForce); // Apply Damage
								}
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

		log::info("Foot event True");

		if (Runtime::GetBool("GtsNPCEffectImmunityToggle") && giant->formID == 0x14 && tiny->IsPlayerTeammate()) {
			return;
		}
		if (Runtime::GetBool("GtsNPCEffectImmunityToggle") && giant->IsPlayerTeammate() && tiny->IsPlayerTeammate()) {
			return;
		} if (Runtime::GetBool("GtsPCEffectImmunityToggle") && tiny->formID == 0x14) {
			return;
		}	
		
		log::info("Underfoot event: {} stepping on {} with force {}", giant->GetDisplayFullName(), tiny->GetDisplayFullName(), force);

		float giantSize = get_visual_scale(giant);
		bool hasSMT = Runtime::HasMagicEffect(giant, "SmallMassiveThreat");
		if (hasSMT) {
			giantSize += 8.0;
		}

		float tinySize = get_visual_scale(tiny);

		float movementFactor = 1.0;
		
		if (giant->IsSneaking()) {
			movementFactor *= 0.5;
		}
		if (giant->IsSprinting()) {
			movementFactor *= 1.5;
		}
		if (evt.footEvent == FootEvent::JumpLand) {
			movementFactor *= 3.0;
		}

		float sizeRatio = giantSize/tinySize * movementFactor;
		float knockBack = LAUNCH_KNOCKBACK  * giantSize * movementFactor * force;
		const float UNDERFOOT_FORCE = 0.60;

		if (force > UNDERFOOT_FORCE && sizeRatio >= 2.5) { // If under the foot
			DoSizeDamage(giant, tiny, movementFactor, force);
			if (sizeRatio >= 4.0) {
				PushActorAway(giant, tiny, knockBack);
			}
		} else if (!SizeManager::IsLaunching(tiny) && force <= UNDERFOOT_FORCE) {
			if (Runtime::HasPerkTeam(giant, "LaunchPerk")) {
				if (sizeRatio >= 8.0) {
					// Launch
					SizeManager::GetSingleton().GetLaunchData(tiny).lastLaunchTime = Time::WorldTimeElapsed();
					if (Runtime::HasPerkTeam(giant, "LaunchDamage")) {
						float damage = LAUNCH_DAMAGE * giantSize * movementFactor * force/UNDERFOOT_FORCE;
						DamageAV(tiny,ActorValue::kHealth, damage);
						if (GetAV(tiny, ActorValue::kHealth) < (damage * 0.5)) {
							CrushManager::GetSingleton().Crush(giant, tiny); // Crush if hp is low
						}
					}
					PushActorAway(giant, tiny, knockBack);
					ApplyHavokImpulse(tiny, 0, 0, 100 * movementFactor * giantSize * force, 50 * movementFactor * giantSize * force);
				}
			}
		}
	}
	
	void AccurateDamage::DoSizeDamage(Actor* giant, Actor* tiny, float totaldamage, float mult) { // Applies damage and crushing
		log::info("Trying to do size damage");
		if (!SizeManager::GetSingleton().GetPreciseDamage()) {
			return;
		}
		if (!giant) {
			return;
		} if (!tiny) {
			return;
		} if (Runtime::GetBool("GtsNPCEffectImmunityToggle") && giant->formID == 0x14 && tiny->IsPlayerTeammate()) {
			return;
		}
		if (Runtime::GetBool("GtsNPCEffectImmunityToggle") && giant->IsPlayerTeammate() && tiny->IsPlayerTeammate()) {
			return;
		} if (Runtime::GetBool("GtsPCEffectImmunityToggle") && tiny->formID == 0x14) {
			return;
		}	

		
		auto sizemanager = SizeManager::GetSingleton();
		float giantsize = get_visual_scale(giant);
		float tinysize = get_visual_scale(tiny);
		float highheels = (1.0 + HighHeelManager::GetBaseHHOffset(giant).Length()/200);
		float multiplier = giantsize/tinysize  * highheels;
		float multipliernolimit = giantsize/tinysize  * highheels;
		if (multiplier > 4.0) {
			multiplier = 4.0; // temp fix
		}
		float additionaldamage = 1.0 + sizemanager.GetSizeVulnerability(tiny);
		float normaldamage = std::clamp(sizemanager.GetSizeAttribute(giant, 0) * 0.25, 0.25, 999999.0);
		float highheelsdamage = sizemanager.GetSizeAttribute(giant, 3);
		float sprintdamage = 1.0;
		float falldamage = 1.0;
		float weightdamage = giant->GetWeight()/100 + 1.0;

		SizeModifications(giant, tiny, highheels);

		if (giant->IsSprinting()) {
			sprintdamage = 1.5 * sizemanager.GetSizeAttribute(giant, 1);
		}
		if (totaldamage >= 3.0) {
			falldamage = sizemanager.GetSizeAttribute(giant, 2) * 2.0;
		}

		float result = ((multiplier * 4 * giantsize * 9.0) * totaldamage * 0.12) * (normaldamage * sprintdamage * falldamage) * 0.38 * highheelsdamage * additionaldamage;
		if (giant->IsSneaking()) {
			result *= 0.33;
		}
		
		if (multipliernolimit >= 8.0 && (GetAV(tiny, ActorValue::kHealth) <= (result * weightdamage * mult))) {
			CrushManager::GetSingleton().Crush(giant, tiny);
			log::info("Trying to crush: {}, multiplier: {}", tiny->GetDisplayFullName(), multiplier);
			return;
		}
		DamageAV(tiny, ActorValue::kHealth, result * weightdamage * mult * 0.25);
	}
}