#include "magic/effects/smallmassivethreat.hpp"
#include "managers/damage/CollisionDamage.hpp"
#include "managers/damage/SizeHitEffects.hpp"
#include "managers/damage/LaunchActor.hpp"
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
	const float BASE_CHECK_DISTANCE = 20.0f;


	void SetLinearImpulse(bhkRigidBody* body, const hkVector4& a_impulse)
	{
		using func_t = decltype(&SetLinearImpulse);
		REL::Relocation<func_t> func{ RELOCATION_ID(76261, 78091) };
		return func(body, a_impulse);
	}

	float GetLaunchThreshold(Actor* giant) {
		float threshold = 8.0;
		if (Runtime::HasPerkTeam(giant, "LaunchPerk")) {
			threshold = 5.2;
		}
		return threshold;
	}

	float GetLaunchPower_Object(float sizeRatio) {
		// https://www.desmos.com/calculator/wh0vwgljfl
		SoftPotential launch {
			.k = 1.42,
			.n = 0.62,
			.s = 0.6,
			.a = 0.0,
		};
		float power = soft_power(sizeRatio, launch);
		return power;
	}

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


	void LaunchDecide(Actor* giant, Actor* tiny, float force, float launch_power) {
		auto profiler = Profilers::Profile("Other: Launch Actors Decide");
		if (IsBeingHeld(tiny)) {
			return;
		}
		if (IsBeingGrinded(tiny)) {
			return; // Disallow to launch if we're grinding an actor
		}
		auto& CollisionDamage = CollisionDamage::GetSingleton();

		float DamageMult = 0.6;
		float giantSize = get_visual_scale(giant);

		float startpower = 6.0; // determines default power of launching someone

		float threshold = 6.0;
		float SMT = 1.0;

		bool OwnsPerk = false;


		if (HasSMT(giant)) {
			giantSize += 4.0;
			threshold = 0.8;
			force += 0.20;
		}
		float Adjustment = GetScaleAdjustment(tiny);
		float tinySize = get_visual_scale(tiny) * Adjustment;
		float sizeRatio = giantSize/tinySize;

		float knockBack = LAUNCH_KNOCKBACK * giantSize * force;

		auto& sizemanager = SizeManager::GetSingleton();
		bool IsLaunching = sizemanager.IsLaunching(tiny);
		if (IsLaunching) {
			return;
		}

		if (force >= 0.10) {
			float power = (1.0 * launch_power) / Adjustment;
			if (Runtime::HasPerkTeam(giant, "DisastrousTremor")) {
				DamageMult *= 2.0;
				OwnsPerk = true;
				power *= 1.5;
			}

			sizemanager.GetSingleton().GetLaunchData(tiny).lastLaunchTime = Time::WorldTimeElapsed();

			if (Runtime::HasPerkTeam(giant, "LaunchDamage") && CanDoDamage(giant, tiny)) {
				float damage = LAUNCH_DAMAGE * sizeRatio * force * DamageMult;
				InflictSizeDamage(giant, tiny, damage);
				if (OwnsPerk) { // Apply only when we have DisastrousTremor perk
					update_target_scale(tiny, -(damage / 500) * GetDamageSetting(), SizeEffectType::kShrink);

					if (get_target_scale(tiny) < 0.12/Adjustment) {
						set_target_scale(tiny, 0.12/Adjustment);
					}
				}
			}
			PushActorAway(giant, tiny, 1.0);

			std::string name = std::format("LaunchOther_{}", tiny->formID);

			ActorHandle tinyHandle = tiny->CreateRefHandle();

			TaskManager::RunOnce(name, [=](auto& update){
				if (tinyHandle) {
					auto tinyref = tinyHandle.get().get();
					TESObjectREFR* tiny_is_object = skyrim_cast<TESObjectREFR*>(tinyref.get().get());
					if (tiny_is_object) {
						ApplyHavokImpulse(tiny_is_object, 0, 0, startpower * GetLaunchPower(sizeRatio) * force * power, startpower * GetLaunchPower(sizeRatio) * force * power);
					}
				}
			});
		}
	}

	void LaunchObjects(Actor* giant, std::vector<NiPoint3> footPoints, float maxFootDistance, float power) {
		auto profiler = Profilers::Profile("Other: Launch Objects");
		bool AllowLaunch = Persistent::GetSingleton().launch_objects;
		if (!AllowLaunch) {
			return;
		}

		auto cell = giant->GetParentCell();
		float giantScale = get_visual_scale(giant);

		float start_power = 0.4;

		if (Runtime::HasPerkTeam(giant, "DisastrousTremor")) {
			power *= 1.5;
		}
		if (cell) {
			auto data = cell->GetRuntimeData();
			for (auto object: data.references) {
				auto objectref = object.get();
				if (objectref) {
					Actor* NonRef = skyrim_cast<Actor*>(objectref);
					if (!NonRef) { // we don't want to apply it to actors
						NiPoint3 objectlocation = objectref->GetPosition();
						for (auto point: footPoints) {
							float distance = (point - objectlocation).Length();
							if (distance <= maxFootDistance) {
								float force = 1.0 - distance / maxFootDistance;
								float push = start_power * GetLaunchPower_Object(giantScale) * force * power;
								auto Object1 = objectref->Get3D1(false);
								if (Object1) {
									auto collision = Object1->GetCollisionObject();
									if (collision) {
										auto rigidbody = collision->GetRigidBody();
										if (rigidbody) {
											auto body = rigidbody->AsBhkRigidBody();
											if (body) {
												SetLinearImpulse(body, hkVector4(0, 0, push, push));
											}
										}
									}
								}
							}
						}
					}
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

	void LaunchActor::ApplyLaunch(Actor* giant, float radius, float power, FootEvent kind) {
		if (giant->formID == 0x14 || IsTeammate(giant) || EffectsForEveryone(giant)) {
			if (kind == FootEvent::Left) {
				LaunchActor::GetSingleton().LaunchLeft(giant, radius, power);
			}
			if (kind == FootEvent::Right) {
				LaunchActor::GetSingleton().LaunchRight(giant, radius, power);
			}
			if (kind == FootEvent::Butt) {
				auto ThighL = find_node(giant, "NPC L Thigh [LThg]");
				auto ThighR = find_node(giant, "NPC R Thigh [RThg]");
				if (ThighL && ThighR) {
					LaunchActor::LaunchAtNode(giant, radius, power, ThighL);
					LaunchActor::LaunchAtNode(giant, radius, power, ThighR);
				}
			} else if (kind == FootEvent::Breasts) {
				auto BreastL = find_node(giant, "NPC L Breast");
				auto BreastR = find_node(giant, "NPC R Breast");
				auto BreastL03 = find_node(giant, "L Breast03");
				auto BreastR03 = find_node(giant, "R Breast03");
				if (BreastL03 && BreastR03) {
					LaunchActor::LaunchAtNode(giant, radius, power, BreastL03);
					LaunchActor::LaunchAtNode(giant, radius, power, BreastR03);
				} else if (BreastL && BreastR) {
					LaunchActor::LaunchAtNode(giant, radius, power, BreastL);
					LaunchActor::LaunchAtNode(giant, radius, power, BreastR);
				}
			}
		}
	}
	void LaunchActor::ApplyLaunch(Actor* giant, float radius, float power, NiAVObject* node) {
		LaunchActor::LaunchAtNode(giant, radius, power, node); // doesn't need a check since function below has it
	}

	void LaunchActor::LaunchAtNode(Actor* giant, float radius, float power, NiAVObject* node) {
		auto profiler = Profilers::Profile("Other: Launch Actor Crawl");
		if (giant->formID == 0x14 || IsTeammate(giant) || EffectsForEveryone(giant)) {
			if (!node) {
				return;
			}
			if (!giant) {
				return;
			}
			float giantScale = get_visual_scale(giant);
			float launchdamage = 1.6;

			float SCALE_RATIO = GetLaunchThreshold(giant)/GetMovementModifier(giant);
			if (HasSMT(giant)) {
				SCALE_RATIO = 1.0/GetMovementModifier(giant);;
				giantScale *= 1.5;
			}

			NiPoint3 NodePosition = node->world.translate;

			float maxDistance = BASE_CHECK_DISTANCE * radius * giantScale;
			// Make a list of points to check
			std::vector<NiPoint3> points = {
				NiPoint3(0.0, 0.0, 0.0), // The standard position
			};
			std::vector<NiPoint3> CrawlPoints = {};

			for (NiPoint3 point: points) {
				CrawlPoints.push_back(NodePosition);
			}

			for (auto point: CrawlPoints) {
				if (IsDebugEnabled() && (giant->formID == 0x14 || IsTeammate(giant) || EffectsForEveryone(giant))) {
					DebugAPI::DrawSphere(glm::vec3(point.x, point.y, point.z), maxDistance, 600, {0.0, 0.0, 1.0, 1.0});
				}
			}

			NiPoint3 giantLocation = giant->GetPosition();
			LaunchObjects(giant, CrawlPoints, maxDistance, power);

			for (auto otherActor: find_actors()) {
				if (otherActor != giant) {
					if (!AllowStagger(giant, otherActor)) {
						return;
					}
					float tinyScale = get_visual_scale(otherActor);
					if (giantScale / tinyScale > SCALE_RATIO) {
						NiPoint3 actorLocation = otherActor->GetPosition();
						for (auto point: CrawlPoints) {
							float distance = (point - actorLocation).Length();
							if (distance <= maxDistance) {
								float force = 1.0 - distance / maxDistance;
								LaunchDecide(giant, otherActor, force, power);
							}
						}
					}
				}
			}
		}
	}

	void LaunchActor::LaunchLeft(Actor* giant, float radius, float power) {
		auto profiler = Profilers::Profile("Other: Launch Actor Left");
		if (!giant) {
			return;
		}
		float giantScale = get_visual_scale(giant);
		float SCALE_RATIO = GetLaunchThreshold(giant)/GetMovementModifier(giant);
		if (HasSMT(giant)) {
			SCALE_RATIO = 1.0 / GetMovementModifier(giant);
			giantScale *= 1.5;
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
		}
		if (!leftCalf) {
			return;
		}
		if (!leftToe) {
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
			if (IsDebugEnabled() && (giant->formID == 0x14 || IsTeammate(giant) || EffectsForEveryone(giant))) {
				for (auto point: footPoints) {
					DebugAPI::DrawSphere(glm::vec3(point.x, point.y, point.z), maxFootDistance, 600, {0.0, 0.0, 1.0, 1.0});
				}
			}

			NiPoint3 giantLocation = giant->GetPosition();
			LaunchObjects(giant, footPoints, maxFootDistance, power);

			for (auto otherActor: find_actors()) {
				if (otherActor != giant) {
					if (!AllowStagger(giant, otherActor)) {
						return;
					}
					float tinyScale = get_visual_scale(otherActor);
					if (giantScale / tinyScale > SCALE_RATIO) {
						NiPoint3 actorLocation = otherActor->GetPosition();
						for (auto point: footPoints) {
							float distance = (point - actorLocation).Length();
							if (distance <= maxFootDistance) {
								float force = 1.0 - distance / maxFootDistance;//force += 1.0 - distance / maxFootDistance;
								LaunchDecide(giant, otherActor, force, power);
							}
						}
					}
				}
			}
		}
	}



	void LaunchActor::LaunchRight(Actor* giant, float radius, float power) {
		auto profiler = Profilers::Profile("Other: Launch Actor Right");
		if (!giant) {
			return;
		}
		float giantScale = get_visual_scale(giant);
		float SCALE_RATIO = GetLaunchThreshold(giant)/GetMovementModifier(giant);
		if (HasSMT(giant)) {
			SCALE_RATIO = 1.0 / GetMovementModifier(giant);
			giantScale *= 1.5;
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
			if (IsDebugEnabled() && (giant->formID == 0x14 || IsTeammate(giant) || EffectsForEveryone(giant))) {
				for (auto point: footPoints) {
					DebugAPI::DrawSphere(glm::vec3(point.x, point.y, point.z), maxFootDistance, 600, {0.0, 0.0, 1.0, 1.0});
				}
			}

			NiPoint3 giantLocation = giant->GetPosition();

			LaunchObjects(giant, footPoints, maxFootDistance, power);

			for (auto otherActor: find_actors()) {
				if (otherActor != giant) {
					if (!AllowStagger(giant, otherActor)) {
						return;
					}
					float tinyScale = get_visual_scale(otherActor);
					if (giantScale / tinyScale > SCALE_RATIO) {
						NiPoint3 actorLocation = otherActor->GetPosition();
						for (auto point: footPoints) {
							float distance = (point - actorLocation).Length();
							if (distance <= maxFootDistance) {
								float force = 1.0 - distance / maxFootDistance;//force += 1.0 - distance / maxFootDistance;
								LaunchDecide(giant, otherActor, force, power);
							}
						}
					}
				}
			}
		}
	}
}
