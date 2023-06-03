#include "managers/damage/LaunchActor.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/highheel.hpp"
#include "utils/actorUtils.hpp"
#include "data/runtime.hpp"
#include "UI/DebugAPI.hpp"
#include "scale/scale.hpp"
#include "data/time.hpp"
#include "events.hpp"
#include "node.hpp"

using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace {

	const float LAUNCH_DAMAGE = 2.4f;
	const float LAUNCH_KNOCKBACK = 0.02f;
	const float UNDERFOOT_POWER = 0.50;

	void StaggerOr(Actor* giant, Actor* tiny, float power) {
		if (tiny->IsDead()) {
			return;
		}
		float giantSize = get_visual_scale(giant);
		float tinySize = get_visual_scale(tiny);
		if (HasSMT(giant)) {
			giantSize *= 4.0;
		}
		float sizedifference = giantSize/tinySize;
		int ragdollchance = rand() % 30 + 1.0;
		if (sizedifference >= 3.0) {
			PushActorAway(giant, tiny, power/100); // Always push
			return;
		}
		if (ragdollchance < 30.0/sizedifference && sizedifference >= 1.25 && sizedifference < 3.0) {
			tiny->SetGraphVariableFloat("staggerMagnitude", 100.00f); // Stagger actor
			tiny->NotifyAnimationGraph("staggerStart");
			return;
		} else if (ragdollchance == 30.0) {
			PushActorAway(giant, tiny, power/100); // Push instead
			return;
		}
	}

	void LaunchDecide(Actor* giant, Actor* tiny, float force, float damagebonus) {
		float giantSize = get_visual_scale(giant);
		if (HasSMT(giant)) {
			giantSize *= 4.0;
		}
		float tinySize = get_visual_scale(tiny);
		float sizeRatio = giantSize/tinySize;

		float knockBack = LAUNCH_KNOCKBACK * giantSize * force;

		

		auto& sizemanager = SizeManager::GetSingleton();
		if (force >= UNDERFOOT_POWER && sizeRatio >= 1.49) { // If under the foot
			if (!sizemanager.IsLaunching(tiny)) {
				sizemanager.GetSingleton().GetLaunchData(tiny).lastLaunchTime = Time::WorldTimeElapsed();
				if (Runtime::HasPerkTeam(giant, "LaunchDamage")) {
					float damage = LAUNCH_DAMAGE * giantSize * force * damagebonus;
					DamageAV(tiny, ActorValue::kHealth, damage * 0.25);
				}
				StaggerOr(giant, tiny, knockBack);
				ApplyHavokImpulse(tiny, 0, 0, 50 * giantSize * force, 50 * giantSize * force);
			}
		} else if (!sizemanager.IsLaunching(tiny) && force < UNDERFOOT_POWER && sizeRatio >= 1.49) {
			if (Runtime::HasPerkTeam(giant, "LaunchPerk")) {
				if (sizeRatio >= 6.0) { // Launch
					sizemanager.GetSingleton().GetLaunchData(tiny).lastLaunchTime = Time::WorldTimeElapsed();
					if (Runtime::HasPerkTeam(giant, "LaunchDamage")) {
						float damage = LAUNCH_DAMAGE * giantSize * force * damagebonus;
						DamageAV(tiny, ActorValue::kHealth, damage);
					}
					StaggerOr(giant, tiny, knockBack);
					ApplyHavokImpulse(tiny, 0, 0, 50 * giantSize * force, 50 * giantSize * force);
				}
			}
		}
	}
}

namespace Gts {

	LaunchActor& LaunchActor::GetSingleton() noexcept {
		static LaunchActor instance;
		return instance;
	}

	std::string LaunchActor::DebugName() {
		return "LaunchActor";
	}

	void LaunchActor::ApplyLaunch(Actor* giant, float radius, float damagebonus, std::string_view node) {
		const float BASE_DISTANCE = 70.0; // Checks the distance of the tiny against giant. Should be large to encompass giant's general area
		const float BASE_FOOT_DISTANCE = 40.0; // Checks the distance of foot squishing
		const float SCALE_RATIO = 2.0;
		float bonusscale = 1.0;
		float actualGiantScale = get_visual_scale(giant);
		float giantScale = actualGiantScale;

		if (HasSMT(giant)) {
			giantScale *= 2.0;
		}
		NiPoint3 hhOffset = HighHeelManager::GetHHOffset(giant);
		NiPoint3 hhOffsetbase = HighHeelManager::GetBaseHHOffset(giant);
		float hh = hhOffsetbase[2];
		std::vector<NiPoint3> points = {
			NiPoint3(0.0, hh*0.08, -0.25 +(-hh * 0.25)), // The standard at the foot position
			NiPoint3(-1.6, 7.7 + (hh/70), -0.75 + (-hh * 1.15)), // Offset it forward
			NiPoint3(0.0, (hh/50), -0.25 + (-hh * 1.15)), // Offset for HH
		};
		float maxFootDistance = BASE_FOOT_DISTANCE * giantScale * radius;

		auto bone_TP = find_node(giant, node, false);
		auto bone_FP = find_node(giant, node, true);
		for (auto foot: {bone_TP, bone_FP}) {
			std::vector<NiPoint3> footPoints = {};
			for (NiPoint3 point:  points) {
				footPoints.push_back(foot->world*point);
				if (hhOffset.Length() > 1e-4) {
					footPoints.push_back(foot->world*(point-hhOffset)); // Add HH offsetted version
				}
			}
			for (auto otherActor: find_actors()) {
				if (otherActor != giant) {
					if (otherActor) {
						float tinyScale = get_visual_scale(otherActor);
						if (giantScale / tinyScale > SCALE_RATIO) {
							NiPoint3 actorLocation = otherActor->GetPosition();
							// Check the tiny's nodes against the giant's foot points
							for (auto point: footPoints) {
								float distance = (point - actorLocation).Length();
								if (distance < maxFootDistance) {
									float aveForce = 1.0 - distance / maxFootDistance;
									LaunchDecide(giant, otherActor, aveForce, damagebonus);
									break;
								}
							}
						}
					}
				}
			}
		}
	}
}