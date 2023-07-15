#include "magic/effects/smallmassivethreat.hpp"
#include "managers/damage/AccurateDamage.hpp"
#include "managers/damage/SizeHitEffects.hpp"
#include "managers/damage/LaunchActor.hpp"
#include "managers/RipClothManager.hpp"
#include "managers/ai/aifunctions.hpp"
#include "scale/scalespellmanager.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/CrushManager.hpp"
#include "managers/InputManager.hpp"
#include "magic/effects/common.hpp"
#include "managers/GtsManager.hpp"
#include "managers/Attributes.hpp"
#include "managers/hitmanager.hpp"
#include "managers/highheel.hpp"
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

	const float LAUNCH_DAMAGE = 2.4f;
	const float LAUNCH_KNOCKBACK = 0.02f;
	const float UNDERFOOT_POWER = 0.70;


	void LaunchDecide(Actor* giant, Actor* tiny, float force, float damagebonus) {
		if (IsBeingHeld(tiny)) {
			return;
		}
		log::info("Force is :{}", force);
		auto& accuratedamage = AccurateDamage::GetSingleton();
		float giantSize = get_visual_scale(giant);
		float SMT = 1.0;
		float threshold = 6.0;
		if (HasSMT(giant)) {
			giantSize += 1.0;
			threshold = 0.8;
			force += 0.20;
		}
		float tinySize = get_visual_scale(tiny);
		if (IsDragon(tiny)) {
			tinySize *= 2.6;
		}
		float sizeRatio = giantSize/tinySize;

		float knockBack = LAUNCH_KNOCKBACK * giantSize * force;

		auto& sizemanager = SizeManager::GetSingleton();
		log::info("Trying to push actor");
		if (force >= 0.10 && force < UNDERFOOT_POWER && sizeRatio >= 6.0 / GetMovementModifier(giant)) {
			if (Runtime::HasPerkTeam(giant, "LaunchPerk")) {
				sizemanager.GetSingleton().GetLaunchData(tiny).lastLaunchTime = Time::WorldTimeElapsed();
				if (Runtime::HasPerkTeam(giant, "LaunchDamage")) {
					float damage = LAUNCH_DAMAGE * giantSize * force * damagebonus;
					DamageAV(tiny, ActorValue::kHealth, damage);
				}
				log::info("Pushing actor away");
				PushActorAway(giant, tiny, 2);

				

				ActorHandle tinyHandle = tiny->CreateRefHandle();
				std::string name = std::format("PushOther_{}", tiny->formID);
				const float DURATION = 1.2;

				/*TaskManager::RunOnce(name, [=](auto& update){
					if (tinyHandle) {
						TESObjectREFR* tiny_is_object = skyrim_cast<TESObjectREFR*>(tinyHandle.get().get());
						if (tiny_is_object) {
							ApplyHavokImpulse(tiny_is_object, 0, 0, 150 * sizeRatio, 150 * sizeRatio);
						}
					}
				});	*/

				

				TaskManager::RunFor(name, DURATION, [=](auto& progressData){
					if (tinyHandle) {
						TESObjectREFR* tiny_is_object = skyrim_cast<TESObjectREFR*>(tinyHandle.get().get());
						if (tiny_is_object) {
							hkVector4 coords = hkVector4(0, 0, 150 * sizeRatio, 150 * sizeRatio);
							tiny_is_object->InitHavok();
							tiny_is_object->ApplyCurrent(0.5, coords);
							log::info("Applying Current for {}", tinyHandle.get().get()->GetDisplayFullName());
						}
						//ApplyHavokImpulse(tinyHandle.get().get(), 0, 0, 150 * sizeRatio, 150 * sizeRatio);
					}
					return true;
				});
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

	void LaunchActor::ApplyLaunch(Actor* giant, float radius, float damagebonus, FootEvent kind) {
		if (!Runtime::HasPerkTeam(giant, "LaunchPerk")) {
			return;
		}
		if (kind == FootEvent::Left) {
			LaunchActor::GetSingleton().LaunchLeft(giant, radius, damagebonus);
		} if (kind == FootEvent::Right) {
			LaunchActor::GetSingleton().LaunchRight(giant, radius, damagebonus);
		}
	}

	void LaunchActor::LaunchLeft(Actor* giant, float radius, float damagebonus) {
		if (!giant) {
			return;
		}
		float giantScale = get_visual_scale(giant);
		const float BASE_CHECK_DISTANCE = 34.0;
		const float SCALE_RATIO = 6.0;
		if (HasSMT(giant)) {
			giantScale *= 1.85;
		}

		radius *= 1.0 + GetHighHeelsBonusDamage(giant) * 2.5;

		// Get world HH offset
		NiPoint3 hhOffset = HighHeelManager::GetHHOffset(giant);
		NiPoint3 hhOffsetbase = HighHeelManager::GetBaseHHOffset(giant);

		auto leftFoot = find_node(giant, leftFootLookup);
		auto leftCalf = find_node(giant, leftCalfLookup);
		auto leftToe = find_node(giant, leftToeLookup);
		auto BodyBone = find_node(giant, bodyLookup);
		if (!leftFoot) {
			return;
		}if (!leftCalf) {
			return;
		}if (!leftToe) {
			return;
		}if (!BodyBone) {
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

		float maxFootDistance = BASE_CHECK_DISTANCE * radius * giantScale;
		float hh = hhOffsetbase[2];
		// Make a list of points to check
		std::vector<NiPoint3> points = {
			NiPoint3(0.0, hh*0.08, -0.25 +(-hh * 0.25)), // The standard at the foot position
			NiPoint3(-1.6, 7.7 + (hh/70), -0.75 + (-hh * 1.15)), // Offset it forward
			NiPoint3(0.0, (hh/50), -0.25 + (-hh * 1.15)), // Offset for HH
		};
		std::tuple<NiAVObject*, NiMatrix3> left(leftFoot, leftRotMat);

		for (const auto& [foot, rotMat]: {left}) {
			std::vector<NiPoint3> footPoints = {};
			for (NiPoint3 point: points) {
				footPoints.push_back(foot->world*(rotMat*point));
			}
			if (Runtime::GetBool("EnableDebugOverlay") && (giant->formID == 0x14 || giant->IsPlayerTeammate() || Runtime::InFaction(giant, "FollowerFaction"))) {
				for (auto point: footPoints) {
					DebugAPI::DrawSphere(glm::vec3(point.x, point.y, point.z), maxFootDistance);
				}
			}

			NiPoint3 giantLocation = giant->GetPosition();
			for (auto otherActor: find_actors()) {
				if (otherActor != giant) {
					if (!AllowStagger(giant, otherActor)) {
						return;
					}
					float tinyScale = get_visual_scale(otherActor);
					if (giantScale / tinyScale > SCALE_RATIO/GetMovementModifier(giant)) {
						NiPoint3 actorLocation = otherActor->GetPosition();
						for (auto point: footPoints) {
							float distance = (point - actorLocation).Length();
							if (distance <= maxFootDistance) {
								float force = 1.0 - distance / maxFootDistance;//force += 1.0 - distance / maxFootDistance;
								LaunchDecide(giant, otherActor, force, damagebonus/10);
							}
						}
					}
				}
			}
		}
	}
	


	void LaunchActor::LaunchRight(Actor* giant, float radius, float damagebonus) {
		if (!giant) {
			return;
		}
		float giantScale = get_visual_scale(giant);
		const float BASE_CHECK_DISTANCE = 34.0;
		const float SCALE_RATIO = 6.0;
		if (HasSMT(giant)) {
			giantScale *= 1.85;
		}
		radius *= 1.0 + GetHighHeelsBonusDamage(giant) * 2.5;

		// Get world HH offset
		NiPoint3 hhOffset = HighHeelManager::GetHHOffset(giant);
		NiPoint3 hhOffsetbase = HighHeelManager::GetBaseHHOffset(giant);

		auto rightFoot = find_node(giant, rightFootLookup);
		auto rightCalf = find_node(giant, rightCalfLookup);
		auto rightToe = find_node(giant, rightToeLookup);
		auto BodyBone = find_node(giant, bodyLookup);


		if (!rightFoot) {
			return;
		}
		if (!rightCalf) {
			return;
		}
		if (!rightToe) {
			return;
		}
		if (!BodyBone) {
			return; // CTD protection attempts
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

		float maxFootDistance = BASE_CHECK_DISTANCE * radius * giantScale;
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
			if (Runtime::GetBool("EnableDebugOverlay") && (giant->formID == 0x14 || giant->IsPlayerTeammate() || Runtime::InFaction(giant, "FollowerFaction"))) {
				for (auto point: footPoints) {
					DebugAPI::DrawSphere(glm::vec3(point.x, point.y, point.z), maxFootDistance);
				}
			}

			NiPoint3 giantLocation = giant->GetPosition();
			for (auto otherActor: find_actors()) {
				if (otherActor != giant) {
					if (!AllowStagger(giant, otherActor)) {
						return;
					}
					float tinyScale = get_visual_scale(otherActor);
					if (giantScale / tinyScale > SCALE_RATIO/GetMovementModifier(giant)) {
						NiPoint3 actorLocation = otherActor->GetPosition();
						for (auto point: footPoints) {
							float distance = (point - actorLocation).Length();
							if (distance <= maxFootDistance) {
								float force = 1.0 - distance / maxFootDistance;//force += 1.0 - distance / maxFootDistance;
								LaunchDecide(giant, otherActor, force, damagebonus/10);
							}
						}
					}
				}
			}
		}
	}
}
